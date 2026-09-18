const fs = require('fs');
const path = require('path');
const Grf = require('./grfController');
const configs = require('../config/configs');
const LRUCache = require('../utils/LRUCache');
const logger = require('../utils/logger');
const searchPool = require('../utils/searchPool');
const iconv = require('iconv-lite');

/**
 * Convert mojibake (CP949 bytes interpreted as Latin-1) back to proper Korean Unicode.
 * roBrowser sends paths like "À¯ÀúÀÎÅÍÆäÀÌ½º" which is CP949 bytes of "유저인터페이스"
 * read as ISO-8859-1. We reverse this by encoding as Latin-1 then decoding as CP949.
 */
function decodeMojibake(str) {
  try {
    const latin1Buf = iconv.encode(str, 'iso-8859-1');
    return iconv.decode(latin1Buf, 'cp949');
  } catch (e) {
    return str;
  }
}

const PROJECT_ROOT = path.resolve(__dirname, '..', '..');

/**
 * Top-level directories the client is allowed to read from disk.
 *
 * These are the Ragnarok client asset trees (the same ones .gitignore excludes).
 * The project root itself is NOT a document root: it also holds .env, .git/,
 * logs/ and the server source, none of which may ever be reachable over HTTP.
 */
const SERVABLE_ROOTS = ['data', 'bgm', 'system', 'ai'];

/**
 * Resolve a client-supplied path inside `base`, or return null if it escapes.
 *
 * Rejects NUL bytes, absolute paths (which would make path.resolve discard the
 * base entirely) and any sequence that climbs out of the base with "..".
 */
function resolveContained(base, requestPath) {
  if (typeof requestPath !== 'string' || requestPath.length === 0) return null;
  if (requestPath.includes('\0')) return null;

  const resolved = path.resolve(base, requestPath);
  const relative = path.relative(base, resolved);
  if (relative === '' || relative.startsWith('..') || path.isAbsolute(relative)) {
    return null;
  }
  return resolved;
}

/** True when a contained path sits under one of the servable asset trees. */
function isServable(resolvedPath) {
  const [top] = path.relative(PROJECT_ROOT, resolvedPath).split(/[\\/]/);
  return SERVABLE_ROOTS.includes(top.toLowerCase());
}

// File content cache (5000 files, 1024MB max)
const fileCache = new LRUCache(
  parseInt(process.env.CACHE_MAX_FILES) || 5000,
  parseInt(process.env.CACHE_MAX_MEMORY_MB) || 1024
);

// GRF file index for O(1) lookups: filename → { grfIndex, originalPath }
let fileIndex = new Map();
let indexBuilt = false;

/**
 * Memoized result of listFiles(). Rebuilding it walks every index entry into a Set -- 67 ms for a
 * full data.grf -- and both /list-files and every search need it. Identity matters too: the search
 * worker keeps its copy keyed on this array, and a fresh array each call would re-send 9 MB per
 * query. Cleared whenever the index is rebuilt.
 */
let cachedFileList = null;

// Path mapping for encoding conversion (loaded from path-mapping.json if exists)
let pathMapping = null;
const pathMappingFile = path.join(__dirname, '..', '..', 'path-mapping.json');
if (fs.existsSync(pathMappingFile)) {
  try {
    pathMapping = JSON.parse(fs.readFileSync(pathMappingFile, 'utf-8'));
    logger.debug(`Loaded path mapping: ${Object.keys(pathMapping.paths || {}).length} entries`);
  } catch (e) {
    logger.error('Failed to load path-mapping.json:', e.message);
  }
}

// Missing files log (async write queue)
const missingFilesLog = path.join(__dirname, '..', '..', 'logs', 'missing-files.log');
const missingFilesSet = new Set();
let lastNotificationTime = 0;
const NOTIFICATION_COOLDOWN = 60000; // 1 minute cooldown between notifications

// Async log queue
let logQueue = [];
let logFlushTimer = null;

function flushLogQueue() {
  if (logQueue.length === 0) return;

  const entries = logQueue.splice(0, logQueue.length);
  const logsDir = path.dirname(missingFilesLog);

  try {
    if (!fs.existsSync(logsDir)) {
      fs.mkdirSync(logsDir, { recursive: true });
    }
    fs.appendFileSync(missingFilesLog, entries.join(''));
  } catch (e) {
    logger.error('Failed to write missing file log:', e.message);
  }
}

const Client = {
  path: '',
  data_ini: '',
  grfs: [],
  AutoExtract: configs.CLIENT_AUTOEXTRACT,
  missingFiles: [],

  async init() {
    const startTime = Date.now();
    this.data_ini = path.join(__dirname, '..', '..', configs.CLIENT_RESPATH, configs.CLIENT_DATAINI);

    if (!fs.existsSync(this.data_ini)) {
      logger.error('DATA.INI file not found:', this.data_ini);
      return;
    }

    const dataIniContent = fs.readFileSync(this.data_ini, 'utf-8');
    const dataIni = parseIni(dataIniContent);

    // Check if data section exists and has GRF files configured
    if (!dataIni.data || dataIni.data.length === 0) {
      logger.warn('No GRF files configured in DATA.INI. Add GRF files to [data] section.');
      this.grfs = [];
      return;
    }

    this.grfs = await Promise.all(
      dataIni.data.filter(Boolean).map(async grfPath => {
        const grf = new Grf(path.join(__dirname, '..', '..', configs.CLIENT_RESPATH, grfPath));
        await grf.load();
        return grf;
      })
    );

    // Build file index for O(1) lookups
    this.buildFileIndex();

    const elapsed = Date.now() - startTime;
    logger.info(`Client initialized in ${elapsed}ms (${fileIndex.size.toLocaleString()} files indexed)`);
  },

  /**
   * Build unified file index from all GRFs
   * Maps normalized paths to { grfIndex, originalPath }
   */
  buildFileIndex() {
    const startTime = Date.now();
    fileIndex.clear();
    let mojibakeCount = 0;

    for (let i = 0; i < this.grfs.length; i++) {
      const grf = this.grfs[i];
      if (grf && grf.listFiles) {
        const files = grf.listFiles();
        for (const file of files) {
          // Normalize: lowercase, forward slashes
          const normalized = file.toLowerCase().replace(/\\/g, '/');

          // Only store first occurrence (first GRF has priority)
          if (!fileIndex.has(normalized)) {
            fileIndex.set(normalized, { grfIndex: i, originalPath: file });
          }

          // Also index with backslashes
          const normalizedBackslash = file.toLowerCase().replace(/\//g, '\\');
          if (!fileIndex.has(normalizedBackslash)) {
            fileIndex.set(normalizedBackslash, { grfIndex: i, originalPath: file });
          }

          // Also index the mojibake version of the path (for roBrowser compatibility)
          // roBrowser sends Korean paths as CP949 bytes interpreted as Latin-1
          try {
            const cp949Buf = iconv.encode(file, 'cp949');
            const mojibakePath = iconv.decode(cp949Buf, 'iso-8859-1');
            if (mojibakePath !== file) {
              const normalizedMojibake = mojibakePath.toLowerCase().replace(/\\/g, '/');
              if (!fileIndex.has(normalizedMojibake)) {
                fileIndex.set(normalizedMojibake, { grfIndex: i, originalPath: file });
                mojibakeCount++;
              }
              const mojibakeBackslash = mojibakePath.toLowerCase().replace(/\//g, '\\');
              if (!fileIndex.has(mojibakeBackslash)) {
                fileIndex.set(mojibakeBackslash, { grfIndex: i, originalPath: file });
              }
            }
          } catch (e) {
            // Skip files that can't be encoded
          }
        }
      }
    }
    if (mojibakeCount > 0) {
      logger.debug(`Added ${mojibakeCount} mojibake path mappings for roBrowser compatibility`);
    }

    // The file list and the search worker both cache what the index holds.
    cachedFileList = null;
    searchPool.invalidate();

    // Add path mapping entries to index
    if (pathMapping && pathMapping.paths) {
      for (const [koreanPath, grfPath] of Object.entries(pathMapping.paths)) {
        const normalizedKorean = koreanPath.toLowerCase().replace(/\\/g, '/');
        const normalizedGrf = grfPath.toLowerCase().replace(/\\/g, '/');

        // If we have the GRF path indexed, also index the Korean path
        if (fileIndex.has(normalizedGrf)) {
          const entry = fileIndex.get(normalizedGrf);
          if (!fileIndex.has(normalizedKorean)) {
            fileIndex.set(normalizedKorean, { ...entry, mappedFrom: koreanPath });
          }
        }
      }
    }

    indexBuilt = true;
    const elapsed = Date.now() - startTime;
    logger.debug(`File index built in ${elapsed}ms`);
  },

  /**
   * Get file with pre-computed ETag from cache
   * Returns { data, etag } or null
   */
  getFileCachedETag(filePath) {
    const cacheKey = filePath.toLowerCase();
    return fileCache.get(cacheKey);
  },

  async getFile(filePath) {
    // Check cache first
    const cacheKey = filePath.toLowerCase();
    const cached = fileCache.get(cacheKey);
    if (cached) {
      return cached.data;
    }

    // Normalize paths
    let grfFilePath = filePath.replace(/\//g, '\\');

    // Check local file system first. The path comes straight from the client, so it
    // is only read when it stays inside the project and lands in an asset tree.
    const localPath = resolveContained(PROJECT_ROOT, filePath);
    if (localPath && isServable(localPath) && fs.existsSync(localPath)) {
      try {
        const content = fs.readFileSync(localPath);
        fileCache.set(cacheKey, content);
        return content;
      } catch (e) {
        logger.error(`Error reading local file: ${e.message}`);
      }
    }

    // Check DATA_OVERRIDE_PATH (external data dir with loose files not in GRF)
    if (process.env.DATA_OVERRIDE_PATH) {
      const relativePath = filePath.replace(/^data[\/\\]/, '');
      const overrideBase = path.resolve(PROJECT_ROOT, process.env.DATA_OVERRIDE_PATH);
      const overridePath = resolveContained(overrideBase, relativePath);
      if (overridePath && fs.existsSync(overridePath)) {
        try {
          const content = fs.readFileSync(overridePath);
          fileCache.set(cacheKey, content);
          return content;
        } catch (e) {
          logger.error(`Error reading override file: ${e.message}`);
        }
      }
    }

    // Use file index for O(1) GRF lookup
    const normalizedPath = filePath.toLowerCase().replace(/\\/g, '/');
    const normalizedBackslash = filePath.toLowerCase().replace(/\//g, '\\');

    let indexEntry = fileIndex.get(normalizedPath) || fileIndex.get(normalizedBackslash);

    // Try mojibake decode: convert Latin-1 mojibake back to Korean Unicode
    if (!indexEntry) {
      const decodedPath = decodeMojibake(filePath);
      if (decodedPath !== filePath) {
        const normalizedDecoded = decodedPath.toLowerCase().replace(/\\/g, '/');
        const normalizedDecodedBack = decodedPath.toLowerCase().replace(/\//g, '\\');
        indexEntry = fileIndex.get(normalizedDecoded) || fileIndex.get(normalizedDecodedBack);
      }
    }

    // Try path mapping if not in index
    if (!indexEntry && pathMapping && pathMapping.paths) {
      const mappedPath = pathMapping.paths[grfFilePath] || pathMapping.paths[filePath];
      if (mappedPath) {
        const normalizedMapped = mappedPath.toLowerCase().replace(/\\/g, '/');
        indexEntry = fileIndex.get(normalizedMapped);
      }
    }

    // Fast path: use index
    if (indexEntry) {
      const grf = this.grfs[indexEntry.grfIndex];
      if (grf && grf.getFile) {
        const fileContent = await grf.getFile(indexEntry.originalPath);
        if (fileContent) {
          // Cache the result
          fileCache.set(cacheKey, fileContent);

          // Auto-extract if enabled
          if (this.AutoExtract) {
            this.extractFile(localPath, fileContent);
          }

          return fileContent;
        }
      }
    }

    // Fallback: sequential search (for files not in index)
    const pathsToTry = [grfFilePath];
    if (pathMapping && pathMapping.paths) {
      const mappedPath = pathMapping.paths[grfFilePath] || pathMapping.paths[filePath];
      if (mappedPath) pathsToTry.push(mappedPath);
    }

    for (const grf of this.grfs) {
      if (grf && grf.getFile) {
        for (const tryPath of pathsToTry) {
          const fileContent = await grf.getFile(tryPath);
          if (fileContent) {
            fileCache.set(cacheKey, fileContent);

            if (this.AutoExtract) {
              this.extractFile(localPath, fileContent);
            }

            return fileContent;
          }
        }
      }
    }

    // Log missing file
    this.logMissingFile(filePath, grfFilePath, null);
    return null;
  },

  /**
   * Extract file to local filesystem (async)
   */
  extractFile(localPath, content) {
    setImmediate(() => {
      try {
        const extractDir = path.dirname(localPath);
        if (!fs.existsSync(extractDir)) {
          fs.mkdirSync(extractDir, { recursive: true });
        }
        fs.writeFileSync(localPath, content);
      } catch (e) {
        logger.error(`Failed to extract file: ${e.message}`);
      }
    });
  },

  logMissingFile(requestedPath, grfPath, mappedPath) {
    if (missingFilesSet.has(requestedPath)) return;

    missingFilesSet.add(requestedPath);

    const logEntry = {
      timestamp: new Date().toISOString(),
      requestedPath,
      grfPath,
      mappedPath: mappedPath || null,
    };

    // Add to in-memory list (max 1000 entries)
    this.missingFiles.push(logEntry);
    if (this.missingFiles.length > 1000) {
      this.missingFiles.shift();
    }

    // Queue log entry for async write
    logQueue.push(JSON.stringify(logEntry) + '\n');

    // Flush queue after 1 second of inactivity
    if (logFlushTimer) clearTimeout(logFlushTimer);
    logFlushTimer = setTimeout(flushLogQueue, 1000);

    logger.debug(`File not found: ${grfPath}${mappedPath ? ` (tried: ${mappedPath})` : ''}`);

    // Check if we should send notification
    this.checkNotification();
  },

  checkNotification() {
    const now = Date.now();
    if (now - lastNotificationTime < NOTIFICATION_COOLDOWN) return;
    if (this.missingFiles.length < 10) return;

    lastNotificationTime = now;

    logger.warn(`MISSING FILES ALERT: ${this.missingFiles.length} files not found. Log: ${missingFilesLog}`);
  },

  getMissingFilesSummary() {
    return {
      total: this.missingFiles.length,
      files: this.missingFiles.slice(-50),
      logFile: missingFilesLog,
    };
  },

  getCacheStats() {
    return fileCache.getStats();
  },

  getIndexStats() {
    return {
      totalFiles: fileIndex.size,
      grfCount: this.grfs.length,
      indexBuilt,
    };
  },

  listFiles() {
    // Use index if available for faster response
    if (indexBuilt) {
      if (cachedFileList) return cachedFileList;

      const uniqueFiles = new Set();
      for (const [, entry] of fileIndex) {
        uniqueFiles.add(entry.originalPath);
      }
      cachedFileList = Array.from(uniqueFiles);
      return cachedFileList;
    }

    // Fallback to GRF iteration
    const allFiles = new Set();
    for (const grf of this.grfs) {
      if (grf && grf.listFiles) {
        const files = grf.listFiles();
        files.forEach(file => allFiles.add(file));
      }
    }
    return Array.from(allFiles);
  },

  /**
   * Search the file index with a client-supplied regex.
   *
   * The pattern is attacker-controlled, and a catastrophic one cannot be interrupted mid-evaluation:
   * `^(.+)+#$` against a 30-character path takes seconds and doubles with each extra character,
   * while GRF paths run 40-60. Checking a time budget between candidates does not help, because the
   * cost is inside a single RegExp.test() call.
   *
   * So the matching runs in a worker thread that the caller can terminate on overrun. This keeps the
   * event loop free: the process stays responsive while a hostile pattern burns, and the worker is
   * replaced for the next query.
   *
   * @returns {Promise<string[]>}
   * @throws if the query exceeds its deadline -- the caller should answer 503.
   */
  async search(regex, { limit = 10000, timeBudgetMs = 2000 } = {}) {
    if (!configs.CLIENT_ENABLESEARCH) {
      logger.warn('Search feature is disabled');
      return [];
    }

    const candidates = this.listFiles();
    return searchPool.search(regex.source, regex.flags, candidates, {
      limit,
      timeoutMs: timeBudgetMs,
    });
  },

  /**
   * Warm up cache with frequently accessed files
   */
  async warmCache(patterns = [], limit = 500) {
    const defaultPatterns = [
      // UI and interface (loaded on every session)
      /data\/texture\/À¯ÀúÀÎÅÍÆäÀÌ½º/i,
      /data\/texture\/userinterface/i,
      /loading\//i,
      /cardbmp\//i,
      // Map data (prontera = default spawn)
      /prontera\.gat$/i,
      /prontera\.gnd$/i,
      /prontera\.rsw$/i,
      // Common map formats (altitude, ground, world)
      /\.gat$/i,
      /\.rsw$/i,
      // Player sprites (all classes)
      /data\/sprite\/ÀÎ°£Á·/i,
      /data\/sprite\/인간족/i,
      // Palette files (small, frequently accessed)
      /\.pal$/i,
      // Lua/lub config files (small, loaded early)
      /\.lub$/i,
    ];

    const patternsToUse = patterns.length > 0 ? patterns : defaultPatterns;
    const maxFiles = limit;
    let warmed = 0;
    const startTime = Date.now();

    for (const [, entry] of fileIndex) {
      if (warmed >= maxFiles) break;

      for (const pattern of patternsToUse) {
        if (pattern.test(entry.originalPath)) {
          const grf = this.grfs[entry.grfIndex];
          if (grf && grf.getFile) {
            try {
              const content = await grf.getFile(entry.originalPath);
              if (content) {
                const cacheKey = entry.originalPath.toLowerCase();
                fileCache.set(cacheKey, content);
                warmed++;
              }
            } catch (e) {
              // Skip files that fail to extract
            }
          }
          break;
        }
      }
    }

    const elapsed = Date.now() - startTime;
    logger.info(`Cache warmed with ${warmed} files in ${elapsed}ms`);
    return warmed;
  }
};

function parseIni(data) {
  const regex = {
    section: /^\s*\[\s*([^\]]*)\s*\]\s*$/,
    param: /^\s*([\w\.\-\_]+)\s*=\s*(.*?)\s*$/,
    comment: /^\s*;.*$/
  };
  const value = {};
  const lines = data.split(/[\r\n]+/);
  let section = null;

  lines.forEach(line => {
    if (regex.comment.test(line)) {
      return;
    } else if (regex.param.test(line)) {
      const match = line.match(regex.param);
      const key = parseInt(match[1], 10);
      const val = match[2];
      if (section) {
        if (!value[section]) {
          value[section] = [];
        }
        value[section][key] = val;
      } else {
        if (!value[key]) {
          value[key] = [];
        }
        value[key] = val;
      }
    } else if (regex.section.test(line)) {
      const match = line.match(regex.section);
      section = match[1];
      // Normalizar seção "Data" para lowercase
      if (section.toLowerCase() === 'data') {
        section = 'data';
      }
      if (!value[section]) {
        value[section] = [];
      }
    }
  });

  return value;
}

module.exports = Client;
