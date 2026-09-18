const configs = require('../config/configs');

module.exports = (req, res, next) => {
  if (configs.DEBUG) {
    console.log(`Request: ${req.method} ${req.url}`);
  }
  next();
};
