/**
 * Production gating of the three diagnostic endpoints.
 *
 * /api/missing-files and /api/cache-stats must 404 outside development. /api/health cannot 404 -- load
 * balancers and container health checks call it, and index.html renders it -- so it stays reachable and
 * withholds the reconnaissance detail instead: absolute paths, GRF names, Node and npm versions, every
 * startup error and warning, plus cache and index internals.
 *
 * SCOPE, honestly: these are STRUCTURAL assertions over index.js, not HTTP calls. The handlers are
 * defined inside startServer(), which runs fatal startup validation and therefore needs a real GRF --
 * not something CI can supply. So this catches a gate being deleted or a sensitive key creeping back
 * into the production payload; it does not re-execute the routes.
 *
 * The functional proof was done once against a live server with a real 3.47 GB archive:
 *
 *   development                         production
 *   /api/health        200, 10 keys     200, 4 keys (timestamp,status,hasWarnings,summary)
 *   /api/missing-files 200              404
 *   /api/cache-stats   200              404
 *   /data/.../loading01.jpg 200 image/jpeg 154176 bytes (both modes)
 *   /.env              404              404
 *
 * Making this functional means extracting the health payload into an exported pure function; worth doing
 * if these endpoints grow.
 */
const test = require('node:test');
const assert = require('node:assert');
const fs = require('node:fs');
const path = require('node:path');

const SOURCE = fs.readFileSync(path.resolve(__dirname, '..', 'index.js'), 'utf8');

/** Return the body of an app.get('<route>', ...) handler. */
function handlerBody(route) {
  const start = SOURCE.indexOf(`app.get('${route}'`);
  assert.notStrictEqual(start, -1, `route ${route} is gone from index.js`);
  const rest = SOURCE.slice(start);
  const end = rest.indexOf('\n  });');
  assert.notStrictEqual(end, -1, `could not find the end of the ${route} handler`);
  return rest.slice(0, end);
}

for (const route of ['/api/missing-files', '/api/cache-stats']) {
  test(`${route} answers 404 in production`, () => {
    const body = handlerBody(route);
    assert.match(
      body,
      /if \(IS_PROD\) return res\.status\(404\)/,
      `${route} lost its production gate — it would expose internals to unauthenticated callers`
    );
  });
}

test('/api/health stays reachable in production', () => {
  const body = handlerBody('/api/health');
  assert.match(body, /if \(IS_PROD\)/, '/api/health lost its production branch');
  assert.doesNotMatch(
    body,
    /if \(IS_PROD\) return res\.status\(404\)/,
    '/api/health must not 404 — health checks and load balancers depend on it'
  );
});

test('/api/health withholds every sensitive key in production', () => {
  const body = handlerBody('/api/health');
  const branch = body.slice(body.indexOf('if (IS_PROD)'));
  const production = branch.slice(0, branch.indexOf('});') + 3);

  const keys = [...production.matchAll(/^\s+(\w+):/gm)].map((m) => m[1]);
  assert.deepStrictEqual(
    keys.sort(),
    ['hasWarnings', 'status', 'summary', 'timestamp'],
    'the production payload changed shape'
  );

  for (const sensitive of ['details', 'messages', 'missingFiles', 'cache', 'index', 'esrgan']) {
    assert.ok(
      !keys.includes(sensitive),
      `${sensitive} leaked back into the production /api/health payload`
    );
  }
});

test('the static mount denies dot-files', () => {
  assert.match(
    SOURCE,
    /express\.static\([^)]*dotfiles:\s*'deny'/,
    "express.static lost dotfiles:'deny' — the served checkout's .git/ and .env become downloadable"
  );
});
