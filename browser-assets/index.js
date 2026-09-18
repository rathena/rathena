require('dotenv').config();

const express = require('express');
const http = require('http');
const path = require('path');
const cors = require('cors');
const net = require('net');
const compression = require('compression');
const logger = require('./src/utils/logger');
const StartupValidator = require('./src/validators/startupValidator');

const app = express();
const server = http.createServer(app);
const port = process.env.PORT || 3338;
const routes = require('./src/routes');
const debugMiddleware = require('./src/middlewares/debugMiddleware');
const createRawImportMiddleware = require('./src/middlewares/rawImportMiddleware');

const CLIENT_PUBLIC_URL = process.env.CLIENT_PUBLIC_URL || 'http://localhost:8000';
const ENABLE_WSPROXY = process.env.ENABLE_WSPROXY === 'true';
const ENABLE_STATIC_SERVE = process.env.ENABLE_STATIC_SERVE === 'true';
const ESRGAN_ENABLED = process.env.ESRGAN_ENABLED === 'true';
const ESRGAN_CACHE_DIR = process.env.ESRGAN_CACHE_DIR || './upscaled_cache';
const ROBROWSER_PATH = process.env.ROBROWSER_PATH || '../roBrowserLegacy';
const IS_PROD = process.env.NODE_ENV === 'production';

// Global variable to store validation status
let validationStatus = null;

// Game asset extensions that benefit from compression
const COMPRESSIBLE_GAME_EXTENSIONS = /\.(spr|act|rsm|gnd|gat|rsw|str|bmp|tga|pal|lub|lua|txt|xml)$/i;

// Main startup function
async function startServer() {
  // Run startup validation
  logger.info(`Starting roBrowser Remote Client... [${IS_PROD ? 'production' : 'development'}]\n`);

  const validator = new StartupValidator();
  const results = await validator.validateAll();

  // Store status for API endpoint
  validationStatus = validator.getStatusJSON();

  // Print report (verbose in dev, silent in prod unless errors)
  if (IS_PROD) {
    if (!results.success) {
      validator.printReport(results);
    }
  } else {
    validator.printReport(results);
  }

  // If there are fatal errors, exit
  if (!results.success) {
    logger.error('Server cannot start due to configuration errors.');
    logger.error('Run "npm run doctor" for a full diagnosis.\n');
    process.exit(1);
  }

  // CORS setup - allow all localhost variations
  const corsOptions = {
    origin: [
      CLIENT_PUBLIC_URL,
      'http://localhost:8000',
      'http://127.0.0.1:8000',
      'http://localhost:8080',
      'http://127.0.0.1:8080',
      'http://localhost:3338',
      'http://127.0.0.1:3338',
    ],
    methods: ['GET', 'POST', 'PUT', 'PATCH', 'DELETE', 'OPTIONS', 'HEAD'],
    credentials: true,
  };

  app.use(cors(corsOptions));
  app.use(express.json());
  app.use(express.urlencoded({ extended: true }));

  // Compression middleware - compresses text AND binary game assets
  app.use(compression({
    threshold: 1024,
    filter: (req, res) => {
      // Compress game assets (SPR, RSM, GND, etc.) that are highly compressible
      if (COMPRESSIBLE_GAME_EXTENSIONS.test(req.path)) {
        return true;
      }
      // Default compression filter for text/json/etc
      return compression.filter(req, res);
    }
  }));

  // Debug middleware only in development
  if (!IS_PROD) {
    app.use(debugMiddleware);
  }

  // ESRGAN upscaling middleware - serves upscaled assets from disk cache
  // Plugin: @chicowall/robrowser-esrgan (optional dependency, installed from GitHub)
  let esrganInstance = null;
  if (ESRGAN_ENABLED) {
    // Resolve before requiring: require.resolve() does not execute the module, so a
    // MODULE_NOT_FOUND here can only mean the plugin itself is absent. Loading it is
    // then left unguarded on purpose -- a broken install must surface as a real error,
    // not be silently downgraded to "not installed".
    let pluginPath = null;
    try {
      pluginPath = require.resolve('@chicowall/robrowser-esrgan');
    } catch (err) {
      if (err.code !== 'MODULE_NOT_FOUND') throw err;
      logger.warn('ESRGAN_ENABLED is set but @chicowall/robrowser-esrgan is not installed.');
      logger.warn('Install it with: npm install github:FranciscoWallison/robrowser-esrgan');
      logger.warn('Continuing without upscaling.\n');
    }

    if (pluginPath) {
      const createEsrganMiddleware = require(pluginPath);
      const cachePath = path.resolve(__dirname, ESRGAN_CACHE_DIR);
      esrganInstance = await createEsrganMiddleware({ cacheDir: cachePath, logger });
      app.use(esrganInstance.middleware);
    }
  }

  // Validation status endpoint (JSON for frontend).
  //
  // Unlike the two diagnostic endpoints below this one cannot 404 in production:
  // load balancers and container health checks call it. So it stays reachable but
  // answers with liveness only, withholding the reconnaissance detail -- absolute
  // paths, GRF names, Node/npm versions, cache and index internals -- that the
  // full payload carries.
  app.get('/api/health', (req, res) => {
    const status = validationStatus || {};

    if (IS_PROD) {
      return res.json({
        timestamp: status.timestamp,
        status: status.status,
        hasWarnings: status.hasWarnings,
        summary: status.summary,
      });
    }

    const Client = require('./src/controllers/clientController');
    const missingInfo = Client.getMissingFilesSummary ? Client.getMissingFilesSummary() : null;
    const cacheStats = Client.getCacheStats ? Client.getCacheStats() : null;
    const indexStats = Client.getIndexStats ? Client.getIndexStats() : null;

    res.json({
      ...status,
      missingFiles: missingInfo,
      cache: cacheStats,
      index: indexStats,
      esrgan: esrganInstance ? esrganInstance.getStats() : { enabled: false },
    });
  });

  // Missing files endpoint (diagnostics: dev-only, hidden in production to avoid
  // leaking file structure/index internals to unauthenticated callers)
  app.get('/api/missing-files', (req, res) => {
    if (IS_PROD) return res.status(404).end();
    const Client = require('./src/controllers/clientController');
    const summary = Client.getMissingFilesSummary ? Client.getMissingFilesSummary() : { total: 0, files: [] };
    res.json(summary);
  });

  // Cache stats endpoint (diagnostics: dev-only, same rationale as above)
  app.get('/api/cache-stats', (req, res) => {
    if (IS_PROD) return res.status(404).end();
    const Client = require('./src/controllers/clientController');
    res.json({
      cache: Client.getCacheStats ? Client.getCacheStats() : null,
      index: Client.getIndexStats ? Client.getIndexStats() : null,
    });
  });

  // Serve roBrowserLegacy static files (replaces live-server)
  if (ENABLE_STATIC_SERVE) {
    const roBrowserAbsPath = path.resolve(__dirname, ROBROWSER_PATH);
    logger.debug(`Static serve enabled: ${roBrowserAbsPath}`);

    // Handle Vite-style ?raw imports (must come before express.static)
    app.use(createRawImportMiddleware(roBrowserAbsPath));

    // dotfiles: 'deny' keeps .git/ and .env out of reach. The doc root is the
    // roBrowserLegacy checkout itself (the raw-import middleware above serves its
    // src/ as ES modules), so without this the whole repository is downloadable.
    app.use(express.static(roBrowserAbsPath, { dotfiles: 'deny' }));
  }

  // API routes (GRF file serving, search, etc.)
  app.use('/', routes);

  // Embedded WebSocket proxy (replaces standalone wsproxy)
  if (ENABLE_WSPROXY) {
    const WebSocket = require('ws');
    const wss = new WebSocket.Server({ noServer: true });

    // Allowed rAthena targets (security: only explicitly listed game servers).
    // Override via WS_ALLOWED_TARGETS (comma-separated host:port) for deployments
    // that cannot use host networking (Kubernetes, Docker Desktop on macOS/Windows,
    // remote rAthena hosts).  The localhost-only default is preserved when the
    // variable is absent or empty.
    const ALLOWED_TARGETS = process.env.WS_ALLOWED_TARGETS
      ? process.env.WS_ALLOWED_TARGETS.split(',').map(s => s.trim())
      : [
          '127.0.0.1:6900',  // Login
          '127.0.0.1:6121',  // Char
          '127.0.0.1:5121',  // Map
        ];

    server.on('upgrade', (req, socket, head) => {
      if (req.url.startsWith('/ws/')) {
        wss.handleUpgrade(req, socket, head, (ws) => {
          wss.emit('connection', ws, req);
        });
      } else {
        socket.destroy();
      }
    });

    wss.on('connection', (ws, req) => {
      // Strip the /ws/ prefix to get "host:port"
      // Use slice (not replace) so a misconfigured socketProxy with no /ws path
      // produces an obviously-invalid target rather than a partial match.
      const target = req.url.slice('/ws/'.length);

      // Validate target format before allowlist check
      const colonIdx = target.lastIndexOf(':');
      const host = colonIdx !== -1 ? target.slice(0, colonIdx) : '';
      const targetPort = colonIdx !== -1 ? parseInt(target.slice(colonIdx + 1), 10) : NaN;

      if (!host || !Number.isInteger(targetPort) || targetPort < 1 || targetPort > 65535) {
        logger.warn(`WS proxy rejected malformed target: "${target}"`);
        ws.close();
        return;
      }

      logger.info(`WS attempt: ${target}`);

      if (!ALLOWED_TARGETS.includes(target)) {
        logger.warn(`WS proxy blocked: ${target} (allowed: ${ALLOWED_TARGETS.join(', ')})`);
        ws.close();
        return;
      }

      logger.info(`WS proxy: connecting to ${target}`);
      const tcp = net.connect(targetPort, host);
      tcp.setNoDelay(true);

      // Buffer messages received before the TCP connection is established.
      // roBrowser sends the first game packet synchronously in its onopen handler,
      // which races with net.connect()'s async 'connect' event. Without buffering,
      // packets arriving before 'connect' fires are silently dropped.
      const MAX_PENDING = 64;
      const pending = [];
      let connected = false;

      // Single cleanup guard: ensures tcp and ws are torn down exactly once
      // regardless of which side closes first or whether an error occurs.
      // Prevents double tcp.end() and misleading "client closed" log on errors.
      let cleaned = false;
      const cleanup = (reason) => {
        if (cleaned) return;
        cleaned = true;
        logger.info(`WS proxy: closed ${target} (${reason})`);
        if (!tcp.destroyed) tcp.destroy();
        if (ws.readyState === WebSocket.OPEN) ws.close();
      };

      tcp.on('connect', () => {
        connected = true;
        logger.info(`WS proxy: connected  to ${target}`);
        pending.splice(0).forEach(d => tcp.write(d));
      });

      ws.on('message', (data) => {
        if (connected) {
          tcp.write(data);
        } else if (pending.length < MAX_PENDING) {
          pending.push(data);
        } else {
          logger.warn(`WS proxy: pending queue full for ${target}, dropping message`);
        }
      });

      tcp.on('data', (data) => {
        if (ws.readyState === WebSocket.OPEN) ws.send(data);
      });

      ws.on('close', () => cleanup('client closed'));
      ws.on('error', (err) => cleanup(`client error: ${err.message}`));
      tcp.on('close', () => cleanup('server closed'));
      tcp.on('error', (err) => cleanup(`server error: ${err.message}`));
    });

    logger.info(`WebSocket proxy enabled on /ws/ (allowed: ${ALLOWED_TARGETS.join(', ')})`);
  }

  server.listen(port, process.env.HOST || '127.0.0.1', async () => {
    logger.info(`Server ready on http://localhost:${port}` +
      (ENABLE_STATIC_SERVE ? ` | Game: http://localhost:${port}/applications/pwa/index.html` : '') +
      (ENABLE_WSPROXY ? ` | WS Proxy: /ws/` : ''));

    // Cache warm-up (runs after server is ready, non-blocking)
    if (process.env.CACHE_WARM_UP === 'true') {
      const warmLimit = parseInt(process.env.CACHE_WARM_UP_LIMIT) || 500;
      logger.debug(`Warming cache (up to ${warmLimit} files)...`);
      const Client = require('./src/controllers/clientController');
      Client.warmCache([], warmLimit).catch(err => {
        logger.error('Cache warm-up error:', err.message);
      });
    }
  });
}

// Start server
startServer().catch((error) => {
  logger.error('Fatal error while starting server:', error);
  process.exit(1);
});
