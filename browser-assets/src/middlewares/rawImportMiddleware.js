/**
 * Raw Import & Import Rewrite Middleware
 *
 * Two responsibilities for ES module compatibility without Vite:
 *
 * 1. **?raw handler**: Wraps file content (HTML, CSS, GLSL shaders) as JS modules
 *    when the `?raw` query parameter is present.
 *
 * 2. **Import rewriter**: Rewrites bare/alias specifiers in JS files under `src/`
 *    to absolute paths. This is necessary because Web Workers don't inherit
 *    the page's import map, so `import X from 'Core/X.js'` must become
 *    `import X from '/src/Core/X.js'`.
 */

const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

/**
 * Import alias map — mirrors the Vite config resolve.alias and package.json deps.
 * Directory aliases use trailing slash; bare specifiers map to specific files.
 */
const IMPORT_ALIASES = {
  // Bare npm specifiers
  'jquery':  '/src/Vendors/jquery-1.9.1.js',
  'bson':    '/node_modules/bson/lib/bson.mjs',
  'lodash':  '/node_modules/lodash-es/lodash.default.js',
  // Subpath export "./wasm" of granny-ro-js, used by src/Loaders/GR2Loader.js and
  // the GR2 renderers. The browser cannot read package.json "exports", so the
  // subpath is spelled out here.
  'granny-ro-js/wasm': '/node_modules/granny-ro-js/dist/granny-ro.wasm.esm.js',
  //
  // NOT mappable: 'rijndael-js', imported by src/Utils/Rijndael.js. That package
  // ships CommonJS only (main: index.js, no "module"/"exports", no type: module),
  // so no static path makes it loadable as an ES module -- serving raw sources
  // cannot substitute for the CJS-to-ESM conversion a bundler performs. Anything
  // reaching Utils/Rijndael.js, which means the login flow, still needs a build.
  // Directory aliases (Vite resolve.alias)
  'App/':         '/src/App/',
  'Audio/':       '/src/Audio/',
  'Controls/':    '/src/Controls/',
  'Core/':        '/src/Core/',
  'DB/':          '/src/DB/',
  'Engine/':      '/src/Engine/',
  'Loaders/':     '/src/Loaders/',
  'Network/':     '/src/Network/',
  'Plugins/':     '/src/Plugins/',
  'Preferences/': '/src/Preferences/',
  'Renderer/':    '/src/Renderer/',
  'UI/':          '/src/UI/',
  'Utils/':       '/src/Utils/',
  'Vendors/':     '/src/Vendors/',
};

// Pre-compute sorted alias keys (longest first for correct matching)
const ALIAS_KEYS = Object.keys(IMPORT_ALIASES).sort((a, b) => b.length - a.length);

/**
 * Regex that matches ES module import/export specifiers.
 *
 * Matches:
 *   import X from 'specifier'        (static default/named import)
 *   import { X } from "specifier"    (static named import)
 *   export { X } from 'specifier'    (re-export)
 *   import('specifier')              (dynamic import)
 */
const STATIC_IMPORT_RE = /((?:import|export)\s+[\s\S]*?\s+from\s+)(["'])((?:(?!\2).)+)\2/g;
const DYNAMIC_IMPORT_RE = /(import\s*\(\s*)(["'])((?:(?!\2).)+)\2/g;

// Cache for rewritten JS files: path → { content, etag, mtime }
const _jsCache = new Map();

/**
 * Rewrite bare/alias specifiers in JS source code to absolute paths.
 */
/**
 * Replace a single specifier match if it's a known alias.
 */
function replaceSpecifier(match, prefix, quote, specifier) {
  // Skip relative and absolute paths — they don't need rewriting
  if (specifier.startsWith('.') || specifier.startsWith('/')) {
    return match;
  }

  for (const alias of ALIAS_KEYS) {
    if (alias.endsWith('/')) {
      // Directory alias: 'Core/' → '/src/Core/'
      if (specifier.startsWith(alias)) {
        const rewritten = IMPORT_ALIASES[alias] + specifier.slice(alias.length);
        return `${prefix}${quote}${rewritten}${quote}`;
      }
    } else {
      // Exact bare specifier: 'jquery' → '/src/Vendors/jquery-1.9.1.js'
      if (specifier === alias) {
        return `${prefix}${quote}${IMPORT_ALIASES[alias]}${quote}`;
      }
    }
  }

  return match;
}

/**
 * Rewrite bare/alias specifiers in JS source code to absolute paths.
 */
function rewriteImports(source) {
  let result = source.replace(STATIC_IMPORT_RE, replaceSpecifier);
  result = result.replace(DYNAMIC_IMPORT_RE, replaceSpecifier);
  return result;
}

/**
 * Creates the combined middleware for a given static root directory.
 *
 * @param {string} rootDir - Absolute path to the roBrowserLegacy directory
 * @returns {function} Express middleware
 */
/**
 * Resolve a request path inside `rootDir`, or return null if it escapes.
 *
 * The boundary separator is the whole point. A bare startsWith(rootDir) also accepts any sibling
 * whose name merely begins with it -- and with rootDir pointing at roBrowserLegacy, that sibling is
 * roBrowserLegacy-RemoteClient-JS, this server's own checkout.
 */
function resolveInside(rootDir, requestPath) {
  const root = path.resolve(rootDir);
  const resolved = path.resolve(path.join(rootDir, requestPath));
  if (resolved !== root && !resolved.startsWith(root + path.sep)) return null;
  return resolved;
}

/** True when any path segment is a dot-entry (.env, .git, ...). */
function hasDotSegment(requestPath) {
  return requestPath.split('/').some((segment) => segment.startsWith('.') && segment.length > 1);
}

function createRawImportMiddleware(rootDir) {
  return function rawImportMiddleware(req, res, next) {
    if (req.method !== 'GET') return next();

    // ── 1. Handle ?raw imports ──
    if ('raw' in req.query) {
      const resolved = resolveInside(rootDir, req.path);
      if (!resolved) {
        return res.status(403).send('Forbidden');
      }

      // ?raw turns any file into a JS module, so dot-entries (.env, .git/) must
      // not be reachable through it.
      if (hasDotSegment(req.path)) {
        return res.status(403).send('Forbidden');
      }

      return fs.readFile(resolved, 'utf8', (err, content) => {
        if (err) {
          return err.code === 'ENOENT'
            ? res.status(404).send('File not found')
            : res.status(500).send('Internal server error');
        }

        const jsModule = `export default ${JSON.stringify(content)};\n`;
        const etag = crypto.createHash('md5').update(jsModule).digest('hex').slice(0, 16);

        const ifNoneMatch = req.headers['if-none-match'];
        if (ifNoneMatch && ifNoneMatch === `"${etag}"`) {
          return res.status(304).end();
        }

        res.set('Content-Type', 'application/javascript; charset=utf-8');
        res.set('ETag', `"${etag}"`);
        res.set('Cache-Control', 'public, max-age=86400');
        res.send(jsModule);
      });
    }

    // ── 2. Rewrite imports in JS files under /src/ and /node_modules/ ──
    if (req.path.endsWith('.js') && (req.path.startsWith('/src/') || req.path.startsWith('/node_modules/'))) {
      // Same containment as the ?raw branch. req.path must start with /src/ or /node_modules/ to
      // reach here, but "/src/../../<sibling>/src/routes/index.js" satisfies that prefix and still
      // resolves outside the root, which served this server's own source verbatim.
      const resolved = resolveInside(rootDir, req.path);
      if (!resolved) {
        return res.status(403).send('Forbidden');
      }

      return fs.stat(resolved, (statErr, stats) => {
        if (statErr) return next(); // Let express.static handle 404

        const mtimeMs = stats.mtimeMs;

        // Check cache
        const cached = _jsCache.get(resolved);
        if (cached && cached.mtime === mtimeMs) {
          const ifNoneMatch = req.headers['if-none-match'];
          if (ifNoneMatch && ifNoneMatch === `"${cached.etag}"`) {
            return res.status(304).end();
          }

          res.set('Content-Type', 'application/javascript; charset=utf-8');
          res.set('ETag', `"${cached.etag}"`);
          res.set('Cache-Control', 'no-cache');
          return res.send(cached.content);
        }

        // Read and rewrite
        fs.readFile(resolved, 'utf8', (readErr, source) => {
          if (readErr) return next();

          const rewritten = rewriteImports(source);
          const etag = crypto.createHash('md5').update(rewritten).digest('hex').slice(0, 16);

          // Cache result
          _jsCache.set(resolved, { content: rewritten, etag, mtime: mtimeMs });

          const ifNoneMatch = req.headers['if-none-match'];
          if (ifNoneMatch && ifNoneMatch === `"${etag}"`) {
            return res.status(304).end();
          }

          res.set('Content-Type', 'application/javascript; charset=utf-8');
          res.set('ETag', `"${etag}"`);
          res.set('Cache-Control', 'no-cache');
          res.send(rewritten);
        });
      });
    }

    // ── 3. Not handled — pass through ──
    next();
  };
}

module.exports = createRawImportMiddleware;
