#!/usr/bin/env node
/**
 * Prepare command - Pre-startup optimization
 *
 * This script prepares everything needed for fast server startup:
 * 1. Validates GRF files
 * 2. Generates path-mapping.json for encoding conversion
 * 3. Pre-builds file index
 * 4. Optionally warms up cache
 *
 * Usage:
 *   npm run setup            # Run all preparation steps
 *   npm run setup:quick      # Quick mode (skip deep validation)
 *
 * Note: this is a manual command. It must NOT be named "prepare" in
 * package.json -- npm treats that name as an install lifecycle hook and would
 * run it on every "npm install", where the client files it checks for do not
 * exist yet.
 */

const fs = require('fs');
const path = require('path');

const args = process.argv.slice(2);
const quickMode = args.includes('--quick');
const verbose = args.includes('--verbose') || args.includes('-v');

console.log('🔧 Preparing roBrowser Remote Client for optimal startup...\n');

const startTime = Date.now();
const results = {
  steps: [],
  errors: [],
  warnings: [],
};

function log(message, type = 'info') {
  const prefix = {
    info: '   ',
    success: ' ✓ ',
    error: ' ✗ ',
    warning: ' ⚠ ',
  }[type] || '   ';

  console.log(`${prefix}${message}`);
}

function addResult(step, status, message, duration) {
  results.steps.push({ step, status, message, duration });
  if (status === 'error') results.errors.push({ step, message });
  if (status === 'warning') results.warnings.push({ step, message });
}

async function step1_validateConfig() {
  const stepStart = Date.now();
  log('Checking configuration...', 'info');

  try {
    require('dotenv').config();
    const configs = require('./src/config/configs');

    // Check required paths
    const dataIniPath = path.join(__dirname, configs.CLIENT_RESPATH, configs.CLIENT_DATAINI);
    if (!fs.existsSync(dataIniPath)) {
      addResult('config', 'error', `DATA.INI not found: ${dataIniPath}`, Date.now() - stepStart);
      return false;
    }

    // Parse DATA.INI to get GRF list
    const dataIni = fs.readFileSync(dataIniPath, 'utf-8');
    const grfFiles = [];
    let inDataSection = false;

    for (const line of dataIni.split(/\r?\n/)) {
      if (/^\s*\[data\]/i.test(line)) {
        inDataSection = true;
        continue;
      }
      if (/^\s*\[/.test(line)) {
        inDataSection = false;
        continue;
      }
      if (inDataSection) {
        const match = line.match(/^\s*\d+\s*=\s*(.+?)\s*$/);
        if (match) grfFiles.push(match[1]);
      }
    }

    if (grfFiles.length === 0) {
      addResult('config', 'warning', 'No GRF files configured in DATA.INI', Date.now() - stepStart);
      return true;
    }

    // Check GRF files exist
    let allExist = true;
    for (const grf of grfFiles) {
      const grfPath = path.join(__dirname, configs.CLIENT_RESPATH, grf);
      if (!fs.existsSync(grfPath)) {
        log(`GRF file not found: ${grf}`, 'error');
        allExist = false;
      } else if (verbose) {
        log(`Found: ${grf}`, 'success');
      }
    }

    if (!allExist) {
      addResult('config', 'error', 'Some GRF files are missing', Date.now() - stepStart);
      return false;
    }

    addResult('config', 'success', `Found ${grfFiles.length} GRF files`, Date.now() - stepStart);
    log(`Found ${grfFiles.length} GRF files`, 'success');
    return true;

  } catch (error) {
    addResult('config', 'error', error.message, Date.now() - stepStart);
    log(error.message, 'error');
    return false;
  }
}

async function step2_generatePathMapping() {
  const stepStart = Date.now();
  log('Generating path mapping for encoding conversion...', 'info');

  try {
    // Check if convert-encoding.mjs exists
    const convertScript = path.join(__dirname, 'tools', 'convert-encoding.mjs');
    if (!fs.existsSync(convertScript)) {
      addResult('pathMapping', 'warning', 'convert-encoding.mjs not found', Date.now() - stepStart);
      log('Skipping (convert-encoding.mjs not found)', 'warning');
      return true;
    }

    // Run the convert script
    const { spawn } = require('child_process');

    return new Promise((resolve) => {
      const child = spawn('node', [convertScript], {
        cwd: __dirname,
        stdio: verbose ? 'inherit' : 'pipe',
      });

      let output = '';
      if (!verbose && child.stdout) {
        child.stdout.on('data', (data) => { output += data.toString(); });
      }
      if (!verbose && child.stderr) {
        child.stderr.on('data', (data) => { output += data.toString(); });
      }

      child.on('close', (code) => {
        if (code === 0) {
          // Check if path-mapping.json was created
          const mappingFile = path.join(__dirname, 'path-mapping.json');
          if (fs.existsSync(mappingFile)) {
            const mapping = JSON.parse(fs.readFileSync(mappingFile, 'utf-8'));
            const count = Object.keys(mapping.paths || {}).length;
            addResult('pathMapping', 'success', `Generated ${count} path mappings`, Date.now() - stepStart);
            log(`Generated ${count} path mappings`, 'success');
          } else {
            addResult('pathMapping', 'success', 'No encoding issues found', Date.now() - stepStart);
            log('No encoding issues found', 'success');
          }
          resolve(true);
        } else {
          addResult('pathMapping', 'warning', 'Path mapping generation had issues', Date.now() - stepStart);
          log('Path mapping generation had issues (non-fatal)', 'warning');
          resolve(true); // Non-fatal
        }
      });

      child.on('error', (err) => {
        addResult('pathMapping', 'warning', err.message, Date.now() - stepStart);
        log(`Warning: ${err.message}`, 'warning');
        resolve(true); // Non-fatal
      });
    });

  } catch (error) {
    addResult('pathMapping', 'warning', error.message, Date.now() - stepStart);
    log(`Warning: ${error.message}`, 'warning');
    return true; // Non-fatal
  }
}

async function step3_buildIndex() {
  const stepStart = Date.now();
  log('Building file index...', 'info');

  try {
    // Initialize client to build index
    const Client = require('./src/controllers/clientController');
    await Client.init();

    const stats = Client.getIndexStats ? Client.getIndexStats() : { totalFiles: 0 };
    addResult('index', 'success', `Indexed ${stats.totalFiles.toLocaleString()} files`, Date.now() - stepStart);
    log(`Indexed ${stats.totalFiles.toLocaleString()} files from ${stats.grfCount} GRFs`, 'success');
    return true;

  } catch (error) {
    addResult('index', 'error', error.message, Date.now() - stepStart);
    log(error.message, 'error');
    return false;
  }
}

async function step4_validateEncoding() {
  if (quickMode) {
    log('Skipping deep encoding validation (quick mode)', 'info');
    addResult('encoding', 'skipped', 'Quick mode', 0);
    return true;
  }

  const stepStart = Date.now();
  log('Validating encoding (this may take a while)...', 'info');

  try {
    const StartupValidator = require('./src/validators/startupValidator');
    const validator = new StartupValidator();

    // Get GRF files
    const configs = require('./src/config/configs');
    const dataIniPath = path.join(__dirname, configs.CLIENT_RESPATH, configs.CLIENT_DATAINI);
    const dataIni = fs.readFileSync(dataIniPath, 'utf-8');
    const grfFiles = [];
    let inDataSection = false;

    for (const line of dataIni.split(/\r?\n/)) {
      if (/^\s*\[data\]/i.test(line)) {
        inDataSection = true;
        continue;
      }
      if (/^\s*\[/.test(line)) {
        inDataSection = false;
        continue;
      }
      if (inDataSection) {
        const match = line.match(/^\s*\d+\s*=\s*(.+?)\s*$/);
        if (match) {
          grfFiles.push(path.join(__dirname, configs.CLIENT_RESPATH, match[1]));
        }
      }
    }

    if (grfFiles.length > 0 && validator.validateEncodingDeep) {
      const encodingResult = await validator.validateEncodingDeep(grfFiles);

      if (encodingResult.issues.length > 0) {
        addResult('encoding', 'warning', `Found ${encodingResult.issues.length} encoding issues`, Date.now() - stepStart);
        log(`Found ${encodingResult.issues.length} files with encoding issues`, 'warning');
        log('Run "npm run convert:encoding" to fix', 'info');
      } else {
        addResult('encoding', 'success', 'No encoding issues', Date.now() - stepStart);
        log('No encoding issues found', 'success');
      }
    } else {
      addResult('encoding', 'skipped', 'No GRF files to validate', Date.now() - stepStart);
      log('No GRF files to validate', 'info');
    }

    return true;

  } catch (error) {
    addResult('encoding', 'warning', error.message, Date.now() - stepStart);
    log(`Warning: ${error.message}`, 'warning');
    return true; // Non-fatal
  }
}

async function step5_createLogsDir() {
  const stepStart = Date.now();
  log('Setting up logging directories...', 'info');

  try {
    const logsDir = path.join(__dirname, 'logs');
    if (!fs.existsSync(logsDir)) {
      fs.mkdirSync(logsDir, { recursive: true });
      addResult('logs', 'success', 'Created logs directory', Date.now() - stepStart);
      log('Created logs directory', 'success');
    } else {
      addResult('logs', 'success', 'Logs directory exists', Date.now() - stepStart);
      log('Logs directory already exists', 'success');
    }
    return true;

  } catch (error) {
    addResult('logs', 'warning', error.message, Date.now() - stepStart);
    log(`Warning: ${error.message}`, 'warning');
    return true; // Non-fatal
  }
}

function printSummary() {
  const totalTime = Date.now() - startTime;

  console.log('\n' + '─'.repeat(50));
  console.log('📊 Preparation Summary');
  console.log('─'.repeat(50));

  for (const step of results.steps) {
    const icon = {
      success: '✓',
      error: '✗',
      warning: '⚠',
      skipped: '○',
    }[step.status] || '?';

    const duration = step.duration > 0 ? ` (${step.duration}ms)` : '';
    console.log(`${icon} ${step.step}: ${step.message}${duration}`);
  }

  console.log('─'.repeat(50));
  console.log(`Total time: ${totalTime}ms`);

  if (results.errors.length > 0) {
    console.log(`\n❌ ${results.errors.length} error(s) found. Server may not start correctly.`);
    return false;
  }

  if (results.warnings.length > 0) {
    console.log(`\n⚠ ${results.warnings.length} warning(s). Server should still work.`);
  }

  console.log('\n✅ Preparation complete! Run "npm start" to start the server.\n');
  return true;
}

async function main() {
  let success = true;

  // Step 1: Validate configuration
  if (!await step1_validateConfig()) {
    success = false;
  }

  // Step 2: Generate path mapping (parallel-safe)
  if (success) {
    await step2_generatePathMapping();
  }

  // Step 3: Build file index
  if (success) {
    if (!await step3_buildIndex()) {
      success = false;
    }
  }

  // Step 4: Deep encoding validation (optional)
  if (success) {
    await step4_validateEncoding();
  }

  // Step 5: Create logs directory
  await step5_createLogsDir();

  // Print summary
  const allOk = printSummary();

  process.exit(allOk ? 0 : 1);
}

main().catch((error) => {
  console.error('\n❌ Fatal error during preparation:', error);
  process.exit(1);
});
