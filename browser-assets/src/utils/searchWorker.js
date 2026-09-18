/**
 * Regex search, run off the main thread.
 *
 * A caller-supplied pattern can backtrack exponentially -- `^(.+)+#$` against a 30-character path
 * takes seconds, and doubles with every additional character, while GRF paths run 40-60 characters.
 * No amount of budget-checking between evaluations helps, because the cost is inside one
 * `RegExp.test()` call that nothing can interrupt.
 *
 * Running it in a worker makes it interruptible: the main thread races every query against a
 * deadline and terminates this thread outright when it overruns. That is the only reliable way to
 * bound an untrusted regex without a linear-time engine.
 */
const { parentPort } = require('node:worker_threads');

/** @type {string[]} The paths to match against, sent once by the main thread. */
let paths = [];

parentPort.on('message', (msg) => {
  if (msg.type === 'seed') {
    paths = msg.paths;
    parentPort.postMessage({ type: 'seeded', count: paths.length });
    return;
  }

  if (msg.type === 'query') {
    // A pattern that does not compile is rejected by the caller before it gets here; compiling
    // again in this thread is cheap and keeps the worker self-contained.
    const regex = new RegExp(msg.pattern, msg.flags);
    const matches = [];

    for (const candidate of paths) {
      if (regex.test(candidate)) {
        matches.push(candidate);
        if (matches.length >= msg.limit) break;
      }
    }

    parentPort.postMessage({ type: 'result', id: msg.id, matches });
  }
});
