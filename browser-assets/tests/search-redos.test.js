/**
 * Regex search must stay interruptible.
 *
 * POST /search compiles a caller-supplied pattern and runs it over every indexed path. A pattern
 * like `^(.+)+#$` backtracks exponentially: measured in this environment it takes ~93 ms against 24
 * characters and roughly doubles per character, while GRF paths run 40-60 -- hours of blocked event
 * loop from one request. And because express.urlencoded is mounted, the form-encoded POST the
 * roBrowser client uses is a CORS *simple* request: no preflight, so any page the victim visits can
 * fire it.
 *
 * An earlier attempt bounded the scan with a wall-clock budget sampled every 1024 candidates. That
 * bounds many cheap evaluations, not one expensive one -- the cost is inside a single
 * RegExp.test() that nothing on that thread can interrupt. Hence the worker, which can be killed.
 */
const test = require('node:test');
const assert = require('node:assert');

const searchPool = require('../src/utils/searchPool');

const PATHS = [
  'data/sprite/monstro/poring_ataque_frente.spr',
  'data/texture/basepic/loading01.jpg',
  'data/texture/basepic/loading00.jpg',
];

// Classic catastrophic pattern, well inside the 256-character cap the route enforces.
const EVIL = '^(.+)+#$';

test('an ordinary pattern still returns its matches', async () => {
  const matches = await searchPool.search('loading', 'i', PATHS, { timeoutMs: 5000 });
  assert.strictEqual(matches.length, 2);
  assert.ok(matches.every((m) => m.includes('loading')));
});

test('the result limit is honoured', async () => {
  const matches = await searchPool.search('loading', 'i', PATHS, { timeoutMs: 5000, limit: 1 });
  assert.strictEqual(matches.length, 1);
});

test('a catastrophic pattern is aborted at the deadline instead of running forever', async () => {
  const started = Date.now();
  await assert.rejects(
    () => searchPool.search(EVIL, 'i', PATHS, { timeoutMs: 1000 }),
    /timed out/
  );
  const elapsed = Date.now() - started;
  assert.ok(elapsed < 5000, `abort took ${elapsed}ms — the deadline is not being enforced`);
});

test('the event loop stays responsive while a hostile pattern burns', async () => {
  let ticks = 0;
  const ticker = setInterval(() => { ticks++; }, 50);

  await assert.rejects(() => searchPool.search(EVIL, 'i', PATHS, { timeoutMs: 1000 }));
  clearInterval(ticker);

  // Without the worker this thread would be wedged inside RegExp.test and tick zero times.
  assert.ok(ticks > 5, `only ${ticks} timer ticks during the attack — the main thread was blocked`);
});

test('searches keep working after a worker is terminated', async () => {
  await assert.rejects(() => searchPool.search(EVIL, 'i', PATHS, { timeoutMs: 500 }));

  const matches = await searchPool.search('loading', 'i', PATHS, { timeoutMs: 5000 });
  assert.strictEqual(matches.length, 2, 'the pool did not recover after terminating the worker');
});

test.after(() => {
  // Release the worker so the test runner can exit.
  searchPool.invalidate();
});
