/**
 * Logger utility that respects NODE_ENV
 *
 * - production: only errors and warnings (minimal output)
 * - development: everything (verbose)
 */
const isDev = process.env.NODE_ENV !== 'production';

const logger = {
  /** Always prints (startup banners, fatal errors) */
  info(...args) {
    console.log(...args);
  },

  /** Only prints in development mode */
  debug(...args) {
    if (isDev) console.log(...args);
  },

  /** Always prints */
  warn(...args) {
    console.warn(...args);
  },

  /** Always prints */
  error(...args) {
    console.error(...args);
  },
};

module.exports = logger;
