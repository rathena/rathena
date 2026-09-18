const express = require('express');
const path = require('path');
const fs = require('fs');
const crypto = require('crypto');
const router = express.Router();
const Client = require('../controllers/clientController');
const configs = require('../config/configs');

// Cache duration settings (in seconds)
const CACHE_DURATIONS = {
  static: 86400,      // 1 day for static game assets
  dynamic: 0,         // No cache for dynamic content
  index: 60,          // 1 minute for index.html
};

// Generate ETag from content
function generateETag(content) {
  return crypto.createHash('md5').update(content).digest('hex').slice(0, 16);
}

// Static game asset extensions
const staticExtensions = [
  '.grf', '.gat', '.rsw', '.gnd', '.rsm', '.str',
  '.spr', '.act', '.pal', '.bmp', '.tga', '.jpg', '.jpeg', '.png', '.gif',
  '.wav', '.mp3', '.ogg',
  '.txt', '.xml', '.lub', '.lua'
];

// Set cache headers based on file type, returns ETag
function setCacheHeaders(res, filePath, content, cachedETag) {
  const ext = path.extname(filePath).toLowerCase();

  if (staticExtensions.includes(ext)) {
    // Use pre-computed ETag from cache when available, otherwise compute
    const etag = cachedETag || generateETag(content);
    res.set('ETag', `"${etag}"`);
    res.set('Cache-Control', `public, max-age=${CACHE_DURATIONS.static}, immutable`);
    res.set('Last-Modified', new Date().toUTCString());
    return etag;
  }

  // Default - no cache
  res.set('Cache-Control', 'no-cache, no-store, must-revalidate');
  return null;
}

// Longest accepted /search pattern. The client sends RegExp.source, which is short
// in practice; the cap keeps a hostile pattern from being arbitrarily complex.
const MAX_FILTER_LENGTH = 256;

// Ceiling on the raw bytes one /batch response may assemble in memory. Bodies are
// base64-encoded on top of this, so the socket sees roughly 4/3 of it.
const MAX_BATCH_BYTES = 32 * 1024 * 1024;

/**
 * Wrap an async handler so a rejected promise reaches Express instead of
 * surfacing as an unhandled rejection. Express 4 does not await handlers, so
 * without this an async throw crashes the process.
 */
function asyncRoute(handler) {
  return (req, res, next) => Promise.resolve(handler(req, res, next)).catch(next);
}

// Check if client has valid cached version
function checkConditionalRequest(req, etag) {
  const ifNoneMatch = req.headers['if-none-match'];
  if (ifNoneMatch && etag && ifNoneMatch === `"${etag}"`) {
    return true;
  }
  return false;
}

// Initialize client on startup
(async () => {
  await Client.init();
})();

router.post('/search', asyncRoute(async (req, res) => {
  const filter = req.body.filter;
  if (!configs.CLIENT_ENABLESEARCH || typeof filter !== 'string' || filter.length === 0) {
    return res.status(400).send('Search feature is disabled or invalid filter');
  }

  if (filter.length > MAX_FILTER_LENGTH) {
    return res.status(400).send(`Filter too long (max ${MAX_FILTER_LENGTH} characters)`);
  }

  let regex;
  try {
    regex = new RegExp(filter, 'i');
  } catch (e) {
    return res.status(400).send('Invalid regular expression');
  }

  try {
    const files = await Client.search(regex);
    res.send(files.join('\n'));
  } catch (err) {
    // The pattern overran its deadline and the worker was terminated. This is the expected
    // outcome for a catastrophic pattern, not a server fault worth a 500.
    res.status(503).send('Search timed out: pattern too expensive');
  }
}));

// Batch file endpoint - fetch multiple files in a single request
router.post('/batch', asyncRoute(async (req, res) => {
  const { files } = req.body;
  if (!Array.isArray(files) || files.length === 0 || files.length > 50) {
    return res.status(400).json({ error: 'Invalid files array (1-50 files)' });
  }

  const results = {};
  let totalBytes = 0;
  let truncated = false;

  await Promise.all(files.map(async (filePath) => {
    if (typeof filePath !== 'string') return;
    try {
      const content = await Client.getFile(filePath);
      if (!content) return;

      // 50 files carry no size limit of their own; without this a caller can ask
      // for the largest assets in the archive and pin them all in memory at once.
      if (totalBytes + content.length > MAX_BATCH_BYTES) {
        truncated = true;
        return;
      }
      totalBytes += content.length;
      results[filePath] = content.toString('base64');
    } catch (e) {
      // Skip files that fail
    }
  }));

  if (truncated) {
    res.set('X-Batch-Truncated', '1');
  }
  res.json(results);
}));

// List files endpoint
router.get('/list-files', asyncRoute(async (req, res) => {
  const files = Client.listFiles();
  res.set('Cache-Control', 'public, max-age=300'); // Cache for 5 minutes
  res.json(files);
}));

// Wildcard route for file serving
router.get('/*', asyncRoute(async (req, res) => {
  const filePath = req.params[0];

  if (/[^\x00-\x7F]/.test(filePath)) {
    console.log('\n=== NON-ASCII ASSET REQUEST ===');
    console.log('originalUrl :', req.originalUrl);
    console.log('url         :', req.url);
    console.log('param       :', filePath);
    console.log(
      'codepoints  :',
      [...filePath].map(c => c.codePointAt(0).toString(16)).join(' ')
    );
    console.log('===============================\n');
  }

  // Serve index.html for root
  if (filePath === '') {
    const indexPath = path.join(__dirname, '..', '..', 'index.html');
    if (!fs.existsSync(indexPath)) {
      return res.status(404).send('index.html not found. Please create an index.html file in the project root.');
    }
    res.set('Cache-Control', `public, max-age=${CACHE_DURATIONS.index}`);
    res.type(path.extname('index.html'));
    return res.send(fs.readFileSync(indexPath, 'utf8'));
  }

  // Try to get pre-computed ETag from cache first (avoids MD5 on every request)
  const cachedEntry = Client.getFileCachedETag ? Client.getFileCachedETag(filePath) : null;

  if (cachedEntry) {
    // Check conditional request using cached ETag before sending data
    if (checkConditionalRequest(req, cachedEntry.etag)) {
      return res.status(304).end();
    }

    // Set content type and cache headers using cached ETag
    res.type(path.extname(filePath));
    setCacheHeaders(res, filePath, cachedEntry.data, cachedEntry.etag);
    return res.send(cachedEntry.data);
  }

  // Cache miss - fetch from GRF or local filesystem
  const fileContent = await Client.getFile(filePath);

  if (!fileContent) {
    res.set('Cache-Control', 'no-store');
    return res.status(404).send('File not found');
  }

  // Set content type
  res.type(path.extname(filePath));

  // Set cache headers and get ETag (computed fresh since not in cache)
  const etag = setCacheHeaders(res, filePath, fileContent, null);

  // Check if client has valid cached version (304 Not Modified)
  if (checkConditionalRequest(req, etag)) {
    return res.status(304).end();
  }

  res.send(fileContent);
}));

module.exports = router;
