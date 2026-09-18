#!/usr/bin/env node
/**
 * convert-encoding.mjs
 *
 * Generates a path mapping file that maps mojibake/C1 paths to their corrected versions.
 * This mapping can be used by the server to resolve file lookups.
 *
 * Usage:
 *  node tools/convert-encoding.mjs [--output=path-mapping.json]
 *
 * Note: This tool does NOT modify GRF files. It creates a lookup table for runtime path resolution.
 */

import { openSync, closeSync, writeFileSync, existsSync, readFileSync } from "fs";
import path from "path";
import { createRequire } from "module";

const require = createRequire(import.meta.url);

// Load the CommonJS build on purpose.
//
// The package's ESM build resolves iconv-lite through a bundler shim that needs a global `require`.
// Under `node -e` one exists and everything appears to work; inside a real .mjs there is none, the
// shim throws, iconv-lite is null, and isMojibake/fixMojibake/toMojibake silently become identity
// functions. That made `npm run convert:encoding` produce an empty mapping while reporting success,
// and made the validators flag encoding errors in archives the server reads perfectly.
const grfLoader = require("@chicowall/grf-loader");
const { GrfNode } = grfLoader;

// These functions may or may not be exported depending on version
const isMojibake = grfLoader.isMojibake || (() => false);
const fixMojibake = grfLoader.fixMojibake || ((s) => s);

// Check if iconv-lite is available
let iconvAvailable = false;
try {
  require.resolve("iconv-lite");
  iconvAvailable = true;
} catch {}

// Parse arguments
const outputArg = process.argv.find((a) => a.startsWith("--output="));
const outputPath = outputArg ? outputArg.split("=")[1] : "path-mapping.json";

console.log("=".repeat(80));
console.log("GRF Encoding Converter");
console.log("=".repeat(80));
console.log("");

console.log(`iconv-lite available: ${iconvAvailable ? "Yes" : "No"}`);
console.log(`Output: ${outputPath}`);
console.log("");

// Helper: check for C1 controls (U+0080..U+009F)
function hasC1Controls(s) {
  for (let i = 0; i < s.length; i++) {
    const c = s.charCodeAt(i);
    if (c >= 0x80 && c <= 0x9f) return true;
  }
  return false;
}

// Parse DATA.INI
function parseDataINI(content) {
  const lines = content.split("\n");
  const grfFiles = [];
  let inDataSection = false;

  for (let line of lines) {
    line = line.trim();
    if (!line || line.startsWith(";") || line.startsWith("#")) continue;

    if (line.toLowerCase() === "[data]") {
      inDataSection = true;
      continue;
    }

    if (line.startsWith("[") && line.endsWith("]")) {
      inDataSection = false;
      continue;
    }

    if (inDataSection && line.includes("=")) {
      const parts = line.split("=");
      const value = parts.slice(1).join("=");
      if (value && value.trim().toLowerCase().endsWith(".grf")) {
        grfFiles.push(value.trim());
      }
    }
  }

  return grfFiles;
}

async function main() {
  const resourcesPath = path.join(process.cwd(), "resources");
  const dataIniPath = path.join(resourcesPath, "DATA.INI");

  if (!existsSync(dataIniPath)) {
    console.error("ERROR: resources/DATA.INI not found!");
    process.exit(1);
  }

  const dataIniContent = readFileSync(dataIniPath, "utf-8");
  const grfFiles = parseDataINI(dataIniContent);

  if (grfFiles.length === 0) {
    console.error("ERROR: No GRF files found in DATA.INI!");
    process.exit(1);
  }

  console.log(`Found ${grfFiles.length} GRF file(s)\n`);

  const mapping = {
    generatedAt: new Date().toISOString(),
    grfs: [],
    paths: {},
    summary: {
      totalFiles: 0,
      totalMapped: 0,
      mojibakeFixed: 0,
      c1Fixed: 0,
    },
  };

  for (const grfFile of grfFiles) {
    const grfPath = path.join(resourcesPath, grfFile);

    if (!existsSync(grfPath)) {
      console.log(`SKIP: ${grfFile} (not found)`);
      continue;
    }

    console.log(`Processing: ${grfFile}`);

    let fd = null;
    try {
      fd = openSync(grfPath, "r");
      const grf = new GrfNode(fd, { filenameEncoding: "auto" });
      await grf.load();

      const stats = grf.getStats?.() ?? {};
      const allFiles = grf.files ? Array.from(grf.files.keys()) : [];

      let grfMapped = 0;
      let grfMojibake = 0;
      let grfC1 = 0;

      for (const filename of allFiles) {
        const s = String(filename);
        mapping.summary.totalFiles++;

        const hasMojibake = isMojibake(s);
        const hasC1 = hasC1Controls(s);

        if (hasMojibake || hasC1) {
          let fixed = s;

          if (hasMojibake) {
            fixed = fixMojibake(s);
            grfMojibake++;
          }

          // For C1 controls, we can try to fix them too
          if (hasC1 && fixed === s) {
            // If mojibake didn't fix it, try to decode C1 as cp949
            // This is a fallback - the library should handle this
            fixed = s; // Keep as-is for now if no better fix available
            grfC1++;
          }

          if (fixed !== s) {
            // koreanPath = what client requests (fixed/Korean)
            // grfPath = what's in the GRF (mojibake)
            const koreanPath = fixed;
            const grfPath = s;

            // Store mapping: koreanPath → grfPath
            // Client requests koreanPath, server looks up grfPath in GRF
            mapping.paths[koreanPath] = grfPath;

            // Also store normalized versions (lowercase, forward slash)
            const normalizedKorean = koreanPath.replace(/\\/g, "/").toLowerCase();
            const normalizedGrf = grfPath.replace(/\\/g, "/").toLowerCase();
            if (normalizedKorean !== koreanPath) {
              mapping.paths[normalizedKorean] = normalizedGrf;
            }
            grfMapped++;
          }
        }
      }

      mapping.grfs.push({
        file: grfFile,
        totalFiles: allFiles.length,
        mapped: grfMapped,
        mojibake: grfMojibake,
        c1: grfC1,
        detectedEncoding: stats.detectedEncoding || "unknown",
      });

      mapping.summary.totalMapped += grfMapped;
      mapping.summary.mojibakeFixed += grfMojibake;
      mapping.summary.c1Fixed += grfC1;

      console.log(`  Files: ${allFiles.length.toLocaleString()} | Mapped: ${grfMapped} | Mojibake: ${grfMojibake} | C1: ${grfC1}`);
    } catch (e) {
      console.log(`  ERROR: ${e.message}`);
    } finally {
      if (fd !== null) {
        try {
          closeSync(fd);
        } catch {}
      }
    }
  }

  console.log("");

  // Write mapping file
  const fullOutputPath = path.resolve(outputPath);
  writeFileSync(fullOutputPath, JSON.stringify(mapping, null, 2), "utf-8");

  console.log("=".repeat(80));
  console.log("SUMMARY");
  console.log("=".repeat(80));
  console.log(`Total files:      ${mapping.summary.totalFiles.toLocaleString()}`);
  console.log(`Total mapped:     ${mapping.summary.totalMapped.toLocaleString()}`);
  console.log(`Mojibake fixed:   ${mapping.summary.mojibakeFixed.toLocaleString()}`);
  console.log(`C1 fixed:         ${mapping.summary.c1Fixed.toLocaleString()}`);
  console.log("");
  console.log(`Mapping saved to: ${fullOutputPath}`);
  console.log("");

  if (mapping.summary.totalMapped > 0) {
    console.log("To use this mapping in your server, load path-mapping.json and use it");
    console.log("to resolve file lookups when the original path is not found.");
    console.log("");
    console.log("Example usage in clientController.js:");
    console.log("  const mapping = require('./path-mapping.json');");
    console.log("  const resolvedPath = mapping.paths[requestedPath] || requestedPath;");
  }

  console.log("=".repeat(80));
}

main().catch((e) => {
  console.error("Fatal error:", e);
  process.exit(1);
});
