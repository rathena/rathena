/**
 * Owns the single worker thread that runs client-supplied regex searches.
 *
 * The worker holds its own copy of the path list (~9 MB for a full data.grf) so a query costs one
 * small message rather than re-sending 170k strings. Every query races a deadline; on overrun the
 * worker is terminated -- which is the whole point, since a catastrophic backtrack cannot be
 * interrupted any other way -- and a fresh one is seeded on the next call.
 */
const path = require('node:path');
const { Worker } = require('node:worker_threads');
const logger = require('./logger');

const WORKER_PATH = path.join(__dirname, 'searchWorker.js');

let worker = null;
let seededWith = null;
let nextQueryId = 1;
const pending = new Map();

/** Reject everything in flight and drop the worker. Used on timeout and on worker death. */
function discardWorker(reason) {
  const dying = worker;
  worker = null;
  seededWith = null;

  for (const [, entry] of pending) {
    clearTimeout(entry.timer);
    entry.reject(new Error(reason));
  }
  pending.clear();

  if (dying) dying.terminate().catch(() => {});
}

function ensureWorker() {
  if (worker) return worker;

  const created = new Worker(WORKER_PATH);
  worker = created;
  created.unref(); // must not keep the process alive on shutdown

  created.on('message', (msg) => {
    if (msg.type !== 'result') return;
    const entry = pending.get(msg.id);
    if (!entry) return; // already timed out
    clearTimeout(entry.timer);
    pending.delete(msg.id);
    entry.resolve(msg.matches);
  });

  // Both handlers check `created === worker` first. A terminated worker still emits 'exit'
  // (with a non-zero code), and by then `worker` may already point at its replacement --
  // acting on that event unconditionally would kill the fresh worker and fail the next query.
  created.on('error', (err) => {
    if (created !== worker) return;
    logger.error('Search worker error:', err.message);
    discardWorker(`search worker failed: ${err.message}`);
  });

  created.on('exit', (code) => {
    if (created !== worker || code === 0) return;
    discardWorker(`search worker exited with code ${code}`);
  });

  return created;
}

/** Send the path list to the worker, once per worker instance. */
function seed(paths) {
  const active = ensureWorker();
  if (seededWith === paths) return;
  active.postMessage({ type: 'seed', paths });
  seededWith = paths;
}

/**
 * Run `pattern` against `paths` in the worker.
 *
 * @returns {Promise<string[]>} the matches, or a rejection if the query overran `timeoutMs`.
 */
function search(pattern, flags, paths, { limit = 10000, timeoutMs = 2000 } = {}) {
  seed(paths);

  const id = nextQueryId++;
  const active = worker;

  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => {
      pending.delete(id);
      logger.warn(`Search aborted after ${timeoutMs}ms: terminating worker (pattern too costly)`);
      discardWorker('search timed out');
      reject(new Error('search timed out'));
    }, timeoutMs);

    // Do not hold the event loop open purely to wait on a search.
    if (typeof timer.unref === 'function') timer.unref();

    pending.set(id, { resolve, reject, timer });
    active.postMessage({ type: 'query', id, pattern, flags, limit });
  });
}

/** Drop the cached path list so the next search re-seeds. Call after the index is rebuilt. */
function invalidate() {
  seededWith = null;
  if (worker) discardWorker('index rebuilt');
}

module.exports = { search, invalidate };
