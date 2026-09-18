// Production launcher - sets NODE_ENV before dotenv loads .env
process.env.NODE_ENV = 'production';
require('./index.js');
