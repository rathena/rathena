#!/usr/bin/env node

/**
 * Doctor Command - Full system diagnosis
 * Run: npm run doctor
 * Run with deep encoding: npm run doctor -- --deep
 */

require('dotenv').config();

const StartupValidator = require('./src/validators/startupValidator');

// Check for --deep flag
const deepEncoding = process.argv.includes('--deep');

async function runDoctor() {
  console.log('\n╔════════════════════════════════════════════════════════════════════════════╗');
  console.log('║                    🏥 roBrowser Remote Client - Doctor                    ║');
  console.log('║                        System Diagnosis                                   ║');
  console.log('╚════════════════════════════════════════════════════════════════════════════╝\n');

  if (deepEncoding) {
    console.log('🔬 Deep encoding validation enabled (this may take a while...)\n');
  }

  const validator = new StartupValidator();
  const results = await validator.validateAll({ deepEncoding });

  // Print detailed report
  validator.printReport(results);

  // Print encoding report if deep validation was done
  if (deepEncoding && results.details.encoding) {
    printEncodingReport(results.details.encoding);
  }

  // If there are errors, show fix instructions
  if (!results.success) {
    console.log('\n📖 FIX GUIDE:\n');

    // Check each error type and provide specific instructions
    const { details } = results;

    // Dependency errors
    if (details.dependencies && !details.dependencies.installed) {
      console.log('1️⃣  DEPENDENCIES NOT INSTALLED:');
      console.log('   Run: npm install');
      if (details.nodeVersion) {
        console.log(`   Versions: Node ${details.nodeVersion.node} | npm ${details.nodeVersion.npm}`);
      }
      console.log('');
    }

    // Environment variable errors
    if (details.env && !details.env.valid) {
      console.log('2️⃣  ENVIRONMENT VARIABLES:');
      console.log('   Create a .env file at the project root:');
      console.log('   ');
      console.log('   PORT=3338');
      console.log('   CLIENT_PUBLIC_URL=http://127.0.0.1:8000');
      console.log('   NODE_ENV=development');
      console.log('');
    }

    // Required files errors
    if (details.files && !details.files.valid) {
      console.log('3️⃣  REQUIRED FILES AND FOLDERS:');
      console.log('   Make sure these exist:');
      console.log('   - resources/');
      console.log('   - resources/DATA.INI');
      console.log('   - At least one .grf file in resources/');
      console.log('');
    }

    // GRF errors
    if (details.grfs && !details.grfs.valid) {
      console.log('4️⃣  INCOMPATIBLE GRF FILES:');
      console.log('   This project only supports GRF version 0x200 with no DES encryption.');
      console.log('');
      console.log('   📦 FIX: Repack with GRF Builder');
      console.log('   ');
      console.log('   1. Download GRF Builder (https://github.com/Tokeiburu/GRFEditor)');
      console.log('   2. Open your .grf file in GRF Builder');
      console.log('   3. Go to: File → Options → Repack type → Decrypt');
      console.log('   4. Click: Tools → Repack');
      console.log('   5. Wait for completion and replace the original file');
      console.log('');
      console.log('   This will convert it to version 0x200 without DES.');
      console.log('');
    }

    console.log('═'.repeat(80));
    console.log('💡 After fixing, run again: npm run doctor');
    console.log('═'.repeat(80) + '\n');

    process.exit(1);
  } else {
    console.log('🎉 System is configured correctly! You can start the server with: npm start\n');

    // Suggest deep encoding if not done
    if (!deepEncoding) {
      console.log('💡 Tip: Run "npm run doctor -- --deep" for detailed encoding analysis\n');
    }
    process.exit(0);
  }
}

/**
 * Print detailed encoding report
 */
function printEncodingReport(encoding) {
  console.log('\n' + '═'.repeat(80));
  console.log('📊 ENCODING VALIDATION REPORT');
  console.log('═'.repeat(80) + '\n');

  console.log(`iconv-lite available: ${encoding.iconvAvailable ? '✅ Yes' : '❌ No'}`);
  console.log('');

  // Summary
  console.log('📈 SUMMARY:');
  console.log(`   Total files:        ${encoding.summary.totalFiles.toLocaleString()}`);
  console.log(`   Bad U+FFFD:         ${encoding.summary.badUfffd.toLocaleString()}`);
  console.log(`   Bad C1 Control:     ${encoding.summary.badC1Control.toLocaleString()}`);
  console.log(`   Mojibake detected:  ${encoding.summary.mojibakeDetected.toLocaleString()}`);
  console.log(`   Needs conversion:   ${encoding.summary.needsConversion.toLocaleString()}`);
  console.log(`   Health:             ${encoding.summary.healthPercent}%`);
  console.log('');

  // Per-GRF details
  for (const grf of encoding.grfs) {
    console.log(`📦 ${grf.file}:`);
    console.log(`   Files: ${grf.totalFiles.toLocaleString()} | Encoding: ${grf.detectedEncoding}`);
    console.log(`   U+FFFD: ${grf.badUfffd} | C1: ${grf.badC1Control} | Mojibake: ${grf.mojibakeDetected}`);

    if (grf.examples.mojibake.length > 0) {
      console.log('   Path mapping (Korean request → GRF path):');
      grf.examples.mojibake.slice(0, 5).forEach((ex) => {
        console.log(`     "${ex.koreanPath}" → "${ex.grfPath}"`);
      });
    }
    console.log('');
  }

  // Files needing conversion
  if (encoding.filesToConvert.length > 0) {
    console.log('🔧 PATH MAPPING TABLE (Korean → GRF):');
    console.log('   When client requests Korean path, lookup GRF path:');
    encoding.filesToConvert.slice(0, 20).forEach((f) => {
      console.log(`   [${f.grf}] "${f.koreanPath}" → "${f.grfPath}"`);
    });
    if (encoding.filesToConvert.length > 20) {
      console.log(`   ... and ${encoding.filesToConvert.length - 20} more`);
    }
    console.log('');
    console.log('💡 Run "npm run convert:encoding" to automatically fix encoding issues');
  }

  console.log('═'.repeat(80) + '\n');
}

// Run doctor
runDoctor().catch((error) => {
  console.error('\n❌ Error while running diagnosis:', error);
  process.exit(1);
});
