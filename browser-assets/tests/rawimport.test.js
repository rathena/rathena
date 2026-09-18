/**
 * Containment in the ?raw handler of rawImportMiddleware.
 *
 * The middleware serves roBrowserLegacy's sources unbundled under ENABLE_STATIC_SERVE, and ?raw wraps
 * whatever it reads in "export default", so anything it can reach becomes downloadable. Its guard used
 * to be resolved.startsWith(path.resolve(rootDir)) with no separator boundary, which also accepted any
 * sibling directory whose name merely begins with the root -- and with rootDir at .../roBrowserLegacy
 * the sibling is .../roBrowserLegacy-RemoteClient-JS, this server's own checkout.
 */
const test = require('node:test');
const assert = require('node:assert');
const path = require('node:path');

const createRawImportMiddleware = require('../src/middlewares/rawImportMiddleware');

// A sibling whose name starts with the root's name is the case the old guard let through.
const ROOT = path.resolve(__dirname, '..', '..', 'roBrowserLegacy');

/**
 * Drive the middleware with a fake req/res and report what it did.
 *
 * The import-rewriter branch answers from an fs.stat callback, so this resolves once the middleware
 * has actually finished rather than reading the outcome synchronously.
 */
function request(urlPath, query = { raw: '' }) {
  const handler = createRawImportMiddleware(ROOT);
  return new Promise((resolve) => {
    const outcome = { status: null, body: null, nexted: false };
    const settle = () => resolve(outcome);
    const res = {
      status(code) { outcome.status = code; return this; },
      send(body) { outcome.body = body; queueMicrotask(settle); return this; },
      set() { return this; },
      type() { return this; },
      end() { queueMicrotask(settle); return this; },
    };
    handler({ method: 'GET', path: urlPath, query, headers: {} }, res, () => {
      outcome.nexted = true;
      settle();
    });
    // Nothing answered synchronously and no callback fired: treat as "did not handle".
    setTimeout(settle, 1500);
  });
}

const MUST_FORBID = [
  ['/../roBrowserLegacy-RemoteClient-JS/.env', 'sibling checkout whose name starts with the root'],
  ['/../roBrowserLegacy-RemoteClient-JS/index.js', 'sibling checkout, source file'],
  ['/../../../Windows/win.ini', 'classic traversal above the root'],
  ['/.env', 'dot-file at the root'],
  ['/.git/config', 'dot-directory at the root'],
  ['/src/../.git/config', 'dot-directory reached through a normal directory'],
];

for (const [urlPath, why] of MUST_FORBID) {
  test(`?raw: 403 for ${urlPath} — ${why}`, async () => {
    const { status } = await request(urlPath);
    assert.strictEqual(status, 403, `expected 403, middleware answered ${status}`);
  });
}

// The import-rewriter branch (no ?raw, .js under /src/ or /node_modules/) had the same weak guard.
// Its prefix test is satisfied by "/src/..." even when the path then climbs out, so this served the
// sibling checkout's source verbatim — 6462 bytes of this project's own src/routes/index.js.
const REWRITER_MUST_FORBID = [
  '/src/../../roBrowserLegacy-RemoteClient-JS/src/routes/index.js',
  '/src/../../roBrowserLegacy-RemoteClient-JS/index.js',
  '/node_modules/../../roBrowserLegacy-RemoteClient-JS/src/controllers/clientController.js',
];

for (const urlPath of REWRITER_MUST_FORBID) {
  test(`rewriter: 403 for ${urlPath}`, async () => {
    const { status, body } = await request(urlPath, {});
    assert.strictEqual(
      status,
      403,
      `escaped the root and answered ${status} with ${body && String(body).length} bytes`
    );
  });
}

test('a request the middleware does not own is passed along untouched', async () => {
  const { nexted, status } = await request('/applications/pwa/index.html', {});
  assert.ok(nexted, 'unrelated requests must fall through to express.static');
  assert.strictEqual(status, null);
});

test('the alias table covers every bare specifier the sources import', () => {
  const source = require('node:fs').readFileSync(
    path.resolve(__dirname, '..', 'src', 'middlewares', 'rawImportMiddleware.js'),
    'utf8'
  );
  // A browser cannot read package.json "exports", so every bare specifier needs a literal path here.
  // rijndael-js is deliberately absent: it ships CommonJS only and cannot be served as an ES module.
  for (const specifier of ['bson', 'lodash', 'granny-ro-js/wasm']) {
    assert.ok(
      source.includes(`'${specifier}'`),
      `IMPORT_ALIASES lost the mapping for '${specifier}' — the module graph will fail to resolve it`
    );
  }
});
