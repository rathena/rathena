import globals from 'globals';
import js from '@eslint/js';
import eslintConfigPrettier from 'eslint-config-prettier';

const sharedRules = {  
  // STYLE (prettier resolve)  
  quotes: ['error', 'single', { avoidEscape: true }],  
  semi: ['error', 'always'],  
  'comma-dangle': ['error', 'never'],  

  // ENGINE FRIENDLY  
  eqeqeq: 'off',  
  'no-implicit-coercion': 'off',  
  'no-plusplus': 'off',  
  'no-fallthrough': 'off',  

  // USELESS RULES  
  'no-useless-assignment': 'off',
  'no-useless-escape': 'off',  
  'no-empty': 'error',  
  'no-redeclare': 'error',  
  'no-constant-condition': 'error',  
  'no-unused-vars': [  
    'warn',  
    {  
      varsIgnorePattern: '^_',  
      argsIgnorePattern: '^_',          
      caughtErrorsIgnorePattern: '^_',  
      args: 'none' 
    }  
  ],  

  // CODE QUALITY (REAL BUGS)  
  curly: ['error', 'all'],  
  'no-eval': 'error',  
  'no-implied-eval': 'error',  
  'no-new-func': 'error',  
  'no-with': 'error',  
  'no-global-assign': 'warn',  
  'no-undef': 'error',  
  'no-unreachable': 'error',  
  'no-new': 'warn',  
  'no-unused-expressions': 'warn',  

  // VARIABLE QUALITY  
  'no-shadow': 'error',  
  'block-scoped-var': 'warn',  
  'no-var': 'error',  
  'prefer-const': 'error',  

  // LESS NOISE FOR LEGACY  
  'no-prototype-builtins': 'off',  
  'no-inner-declarations': 'off',  
  'no-case-declarations': 'error',  

  // DEBUG  
  'no-console': 'off',  
  'no-debugger': 'warn'  
};

export default [
  {
    ignores: [
      'node_modules/',
      'bower_components/',
      'dist/',
      'build/',
      '*.min.js',
      'src/Vendors/**',
      'src/Libs/**',
      'src/lib/**',
      'src/Utils/three.js',
      'src/Utils/gl-matrix.js',
      'test/',
      'tests/',
      '*.test.js',
      '*.spec.js',
      '.cache/',
      '.eslintcache',
      'docs/',
      '*.config.js',
      'webpack.config.js',
      'rollup.config.js'
    ]
  },
  js.configs.recommended,
  {
    files: ['src/**/*.js'],
    languageOptions: {  
      ecmaVersion: 2022,  
      sourceType: 'module',  
      globals: {  
        ...globals.browser,  
        ...globals.es2025, 
        FileReaderSync: 'readonly',  
        importScripts: 'readonly',  
        Buffer: 'readonly',  
        ROConfig: 'readonly',  
        SEEK_CUR: 'readonly',  
        SEEK_SET: 'readonly',  
        SEEK_END: 'readonly'  
      }  
    },  
    rules: sharedRules
  },
  
  {
    files: ['applications/electron/*.js'],
    languageOptions: {
      ecmaVersion: 2022,
      sourceType: 'module',
      globals: {
        ...globals.node,
        ...globals.es2025
      }
    },
    rules: sharedRules
  },

  eslintConfigPrettier  
];
