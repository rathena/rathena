const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");
const zlib = require("zlib");
const { TextDecoder } = require("util");

/**
 * Startup validation system
 * Validates resources, configuration and dependencies before starting the server
 */
class StartupValidator {
  constructor() {
    this.errors = [];
    this.warnings = [];
    this.info = [];
    this.validationResults = {
      grfs: null,
      files: null,
      env: null,
      dependencies: null,
      nodeVersion: null,
      encoding: null, // NEW: detailed encoding validation
    };
  }

  addError(message) {
    this.errors.push(message);
  }

  addWarning(message) {
    this.warnings.push(message);
  }

  addInfo(message) {
    this.info.push(message);
  }

  validateNodeVersion() {
    try {
      const nodeVersion = process.version;
      const npmVersion = execSync("npm --version", { encoding: "utf-8" }).trim();

      this.validationResults.nodeVersion = {
        node: nodeVersion,
        npm: npmVersion,
        valid: true,
      };

      this.addInfo(`Node.js: ${nodeVersion}`);
      this.addInfo(`npm: ${npmVersion}`);

      const majorVersion = parseInt(nodeVersion.replace("v", "").split(".")[0], 10);
      if (majorVersion < 14) {
        this.addWarning(`Node.js version ${nodeVersion} may be too old. Recommended: v14 or newer`);
      }

      return true;
    } catch (error) {
      this.addError(`Failed to check Node.js/npm version: ${error.message}`);
      this.validationResults.nodeVersion = { valid: false, error: error.message };
      return false;
    }
  }

  validateDependencies() {
    const nodeModulesPath = path.join(process.cwd(), "node_modules");
    const packageJsonPath = path.join(process.cwd(), "package.json");

    if (!fs.existsSync(packageJsonPath)) {
      this.addError("package.json not found!");
      this.validationResults.dependencies = { installed: false, reason: "package.json missing" };
      return false;
    }

    if (!fs.existsSync(nodeModulesPath)) {
      const nodeVersion = this.validationResults.nodeVersion;
      const versionInfo = nodeVersion ? `\n  Node.js: ${nodeVersion.node}\n  npm: ${nodeVersion.npm}` : "";
      this.addError(`Dependencies not installed!\nRun: npm install${versionInfo}`);
      this.validationResults.dependencies = { installed: false, reason: "node_modules missing" };
      return false;
    }

    const requiredDeps = ["express", "cors", "@chicowall/grf-loader"];
    const missingDeps = [];

    for (const dep of requiredDeps) {
      const depPath = path.join(nodeModulesPath, dep);
      if (!fs.existsSync(depPath)) missingDeps.push(dep);
    }

    if (missingDeps.length > 0) {
      this.addError(`Missing essential dependencies: ${missingDeps.join(", ")}\nRun: npm install`);
      this.validationResults.dependencies = { installed: false, missingDeps };
      return false;
    }

    this.addInfo("Dependencies installed correctly");
    this.validationResults.dependencies = { installed: true };
    return true;
  }

  /**
   * Validate GRF files (0x200 / 0x300) and validate compacted (zlib) file table.
   * Also diagnoses non-UTF-8 filenames (for path encoding conversion/fallback).
   */
  async validateGrfs() {
    const resourcesPath = path.join(process.cwd(), "resources");
    const dataIniPath = path.join(resourcesPath, "DATA.INI");

    if (!fs.existsSync(dataIniPath)) {
      this.addError("resources/DATA.INI not found!");
      this.validationResults.grfs = { valid: false, reason: "DATA.INI missing" };
      return false;
    }

    const dataIniContent = fs.readFileSync(dataIniPath, "utf-8");
    const grfFiles = this.parseDataINI(dataIniContent);

    if (grfFiles.length === 0) {
      this.addError("No GRF files found in resources/DATA.INI!");
      this.validationResults.grfs = { valid: false, reason: "No GRF files in DATA.INI" };
      return false;
    }

    const grfResults = [];
    let hasInvalidGrf = false;

    for (const grfFile of grfFiles) {
      const grfPath = path.join(resourcesPath, grfFile);

      if (!fs.existsSync(grfPath)) {
        this.addError(`GRF not found: ${grfFile}`);
        grfResults.push({ file: grfFile, exists: false });
        hasInvalidGrf = true;
        continue;
      }

      const validation = await this.validateGrfFormat(grfPath);

      grfResults.push({ file: grfFile, exists: true, ...validation });

      if (!validation.valid) {
        let errorMsg = `Incompatible GRF: ${grfFile}\n`;

        if (validation.version && validation.version !== "unknown") {
          errorMsg += `  ❌ Version: ${validation.version} (expected: 0x200 or 0x300)\n`;
        }

        errorMsg += `  ❌ ${validation.reason}\n`;

        if (validation.fileTable?.ok === false) {
          errorMsg += `  ❌ FileTable(zlib) failed: ${validation.fileTable.reason}\n`;
        }

        errorMsg += `\n  📦 FIX: Repack with GRF Builder:\n`;
        errorMsg += `  1. Download GRF Builder: https://github.com/Tokeiburu/GRFEditor\n`;
        errorMsg += `  2. Open GRF Builder\n`;
        errorMsg += `  3. File → Options → Repack type → Decrypt\n`;
        errorMsg += `  4. Click: Tools → Repack\n`;
        errorMsg += `  5. Wait for completion and replace the original file`;

        this.addError(errorMsg);
        hasInvalidGrf = true;
      } else {
        this.addInfo(`Valid GRF: ${grfFile} (version ${validation.version})`);

        // Path encoding diagnosis
        if (validation.pathEncoding?.encoding === "iso-8859-1") {
          const samples = validation.pathEncoding.invalidUtf8Samples?.length
            ? ` Examples: ${validation.pathEncoding.invalidUtf8Samples.join(" | ")}`
            : "";
          this.addWarning(
            `GRF path encoding: ${grfFile} has non-UTF-8 filenames. Recommend legacy encoding fallback.${samples}`
          );
        }
      }
    }

    this.validationResults.grfs = {
      valid: !hasInvalidGrf,
      files: grfResults,
      count: grfFiles.length,
    };

    return !hasInvalidGrf;
  }

  parseDataINI(content) {
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

  // -------------------- GRF helpers (0x200/0x300 + compacted/zlib) --------------------

  _trimNullTerminatedAscii(buf) {
    const idx = buf.indexOf(0x00);
    const slice = idx >= 0 ? buf.subarray(0, idx) : buf;
    return slice.toString("ascii");
  }

  _safeRead(fd, size, position) {
    const b = Buffer.alloc(size);
    const n = fs.readSync(fd, b, 0, size, position);
    return n === size ? b : b.subarray(0, n);
  }

  _readGrfHeader46(fd) {
    const header = this._safeRead(fd, 46, 0);
    if (header.length < 46) {
      return { ok: false, reason: "Header too small (<46 bytes)" };
    }

    const signature = this._trimNullTerminatedAscii(header.subarray(0, 16));
    if (signature !== "Master of Magic") {
      return { ok: false, reason: `Invalid signature: "${signature}"` };
    }

    const seed = header.readUInt32LE(34) >>> 0;
    const nFiles = header.readUInt32LE(38) >>> 0;
    const version = header.readUInt32LE(42) >>> 0;

    // 0x200 and 0x300 lay out these fields differently. Mirrors the reference
    // implementation in roBrowserLegacy, src/Loaders/GameFile.js.
    //
    // 0x300 stores the table offset as a 64-bit value spanning bytes 30..37 -- so
    // what is the "seed" field in 0x200 is the high half of that offset here --
    // and its file count is literal. 0x200 stores a 32-bit offset and encodes the
    // count as nFiles - seed - 7.
    let tableOffset;
    let fileCount;
    if (version === 0x300) {
      tableOffset = Number(header.readBigUInt64LE(30));
      fileCount = nFiles;
    } else {
      tableOffset = header.readUInt32LE(30) >>> 0;
      fileCount = Math.max(nFiles - seed - 7, 0);
    }

    return {
      ok: true,
      tableOffset,
      seed,
      nFiles,
      fileCount,
      version,
      versionHex: "0x" + version.toString(16).toUpperCase(),
    };
  }

  _inflateFileTable(fd, fileTablePos) {
    const tableHeader = this._safeRead(fd, 8, fileTablePos);
    if (tableHeader.length < 8) {
      return { ok: false, reason: "File table header too small (<8 bytes)" };
    }

    const compressedSize = tableHeader.readUInt32LE(0) >>> 0;
    const uncompressedSize = tableHeader.readUInt32LE(4) >>> 0;

    if (!compressedSize || !uncompressedSize) {
      return { ok: false, reason: "Invalid file table sizes (0)" };
    }

    // guardrail
    const MAX_UNCOMPRESSED = 512 * 1024 * 1024; // 512MB
    if (uncompressedSize > MAX_UNCOMPRESSED) {
      return { ok: false, reason: `Uncompressed file table too large (${uncompressedSize} bytes)` };
    }

    const compressed = this._safeRead(fd, compressedSize, fileTablePos + 8);
    if (compressed.length !== compressedSize) {
      return { ok: false, reason: "Failed reading compressed file table bytes" };
    }

    try {
      const data = zlib.inflateSync(compressed); // zlib stream
      return { ok: true, compressedSize, uncompressedSize, data };
    } catch (e) {
      return { ok: false, reason: `zlib inflate failed: ${e.message}` };
    }
  }

  _scanFileTableNames(tableBuf, fileCount, offsetSize, fileSize, scanLimit) {
    const utf8Decoder = new TextDecoder("utf-8", { fatal: true });

    const metaLen = 4 + 4 + 4 + 1 + offsetSize;
    const maxI = scanLimit > 0 ? Math.min(fileCount, scanLimit) : fileCount;

    let p = 0;
    let inspected = 0;
    let invalidUtf8Count = 0;
    const invalidUtf8Samples = [];
    let parseErrors = 0;
    let offsetOutOfRange = 0;

    const isUtf8 = (bytes) => {
      // fast ASCII
      let hasHigh = false;
      for (let i = 0; i < bytes.length; i += 1) {
        if (bytes[i] >= 0x80) {
          hasHigh = true;
          break;
        }
      }
      if (!hasHigh) return true;
      try {
        utf8Decoder.decode(bytes);
        return true;
      } catch {
        return false;
      }
    };

    const decodeLatin1 = (bytes) => {
      try {
        return new TextDecoder("latin1").decode(bytes);
      } catch {
        return Buffer.from(bytes).toString("latin1");
      }
    };

    for (let i = 0; i < maxI; i += 1) {
      if (p >= tableBuf.length) {
        parseErrors += 1;
        break;
      }

      // filename (null terminated)
      let end = p;
      while (end < tableBuf.length && tableBuf[end] !== 0x00) end += 1;

      if (end >= tableBuf.length) {
        parseErrors += 1;
        break;
      }

      const nameBytes = tableBuf.subarray(p, end);
      p = end + 1;

      if (p + metaLen > tableBuf.length) {
        parseErrors += 1;
        break;
      }

      // compSize(4) compAligned(4) realSize(4) flags(1) offset(4/8)
      const compAligned = tableBuf.readUInt32LE(p + 4) >>> 0;
      const flags = tableBuf[p + 12];

      let offsetVal = 0;
      if (offsetSize === 4) {
        offsetVal = tableBuf.readUInt32LE(p + 13) >>> 0;
      } else {
        // BigInt -> number (best effort)
        try {
          const o = tableBuf.readBigUInt64LE(p + 13);
          offsetVal = o <= BigInt(Number.MAX_SAFE_INTEGER) ? Number(o) : Number.MAX_SAFE_INTEGER;
        } catch {
          offsetVal = Number.MAX_SAFE_INTEGER;
        }
      }

      p += metaLen;

      // only real files
      if (!(flags & 0x01)) continue;

      inspected += 1;

      // basic plausibility check (helps choose correct 0x300 layout)
      if (fileSize > 0) {
        if (offsetVal < 0 || offsetVal >= fileSize) offsetOutOfRange += 1;
        else if (offsetVal + compAligned > fileSize) offsetOutOfRange += 1;
      }

      if (!isUtf8(nameBytes)) {
        invalidUtf8Count += 1;
        if (invalidUtf8Samples.length < 5) invalidUtf8Samples.push(decodeLatin1(nameBytes));
      }
    }

    return {
      inspected,
      invalidUtf8Count,
      invalidUtf8Samples,
      parseErrors,
      offsetOutOfRange,
    };
  }

  _analyzeGrfPathEncoding(fd, headerInfo, fileSize) {
    const scanLimitEnv = process.env.GRF_PATH_SCAN_LIMIT;
    const scanLimit =
      scanLimitEnv && /^\d+$/.test(scanLimitEnv) ? parseInt(scanLimitEnv, 10) : 0; // 0 = full

    // 0x300 carries an extra unknown Int32 between the header and the file table
    // (see roBrowserLegacy, src/Loaders/GameFile.js).
    const fileTablePos =
      headerInfo.tableOffset + 46 + (headerInfo.version === 0x300 ? 4 : 0);
    const table = this._inflateFileTable(fd, fileTablePos);
    if (!table.ok) {
      return {
        ok: false,
        encoding: "unknown",
        reason: table.reason,
      };
    }

    // 0x200: offset32
    // 0x300: try offset32 and offset64, pick best fit
    const scans = [];

    scans.push({
      layout: "offset32",
      offsetSize: 4,
      ...this._scanFileTableNames(table.data, headerInfo.fileCount, 4, fileSize, scanLimit),
    });

    if (headerInfo.version === 0x300) {
      scans.push({
        layout: "offset64",
        offsetSize: 8,
        ...this._scanFileTableNames(table.data, headerInfo.fileCount, 8, fileSize, scanLimit),
      });
    }

    // pick best:
    // 1) more inspected
    // 2) fewer parseErrors
    // 3) fewer offsetOutOfRange (helps for 0x300)
    scans.sort((a, b) => {
      if (b.inspected !== a.inspected) return b.inspected - a.inspected;
      if (a.parseErrors !== b.parseErrors) return a.parseErrors - b.parseErrors;
      return a.offsetOutOfRange - b.offsetOutOfRange;
    });

    const best = scans[0];

    if (!best || best.inspected === 0) {
      return {
        ok: true,
        encoding: "unknown",
        reason: "No file entries inspected (table parse mismatch or empty GRF)",
        table: {
          compressedSize: table.compressedSize,
          uncompressedSize: table.uncompressedSize,
        },
        layoutTried: scans.map((s) => s.layout),
      };
    }

    return {
      ok: true,
      encoding: best.invalidUtf8Count > 0 ? "iso-8859-1" : "utf-8", // meaning: non-UTF-8 vs UTF-8
      layout: best.layout,
      totalFilesInspected: best.inspected,
      invalidUtf8Count: best.invalidUtf8Count,
      invalidUtf8Samples: best.invalidUtf8Samples,
      parseErrors: best.parseErrors,
      offsetOutOfRange: best.offsetOutOfRange,
      table: {
        compressedSize: table.compressedSize,
        uncompressedSize: table.uncompressedSize,
      },
      note:
        best.invalidUtf8Count > 0
          ? "Detected non-UTF-8 filename bytes (legacy encoding)."
          : "All inspected filenames are valid UTF-8.",
    };
  }

  /**
   * Validate GRF file format
   * - Accepts 0x200 and 0x300
   * - Validates compacted file table (zlib) + parses entries (offset32/offset64)
   * - Then tries to load with @chicowall/grf-loader (real compatibility test)
   */
  async validateGrfFormat(grfPath) {
    const { GrfNode } = require("@chicowall/grf-loader");

    let fd = null;
    let testFd = null;

    try {
      fd = fs.openSync(grfPath, "r");
      const stat = fs.fstatSync(fd);
      const fileSize = typeof stat.size === "number" ? stat.size : 0;

      const headerInfo = this._readGrfHeader46(fd);
      if (!headerInfo.ok) {
        return {
          valid: false,
          version: "unknown",
          compatible: false,
          reason: headerInfo.reason,
          fileTable: { ok: false, reason: "Skipped (invalid header)" },
          pathEncoding: { ok: false, encoding: "unknown", reason: "Skipped (invalid header)" },
        };
      }

      // version support for compacted validation
      if (headerInfo.version !== 0x200 && headerInfo.version !== 0x300) {
        return {
          valid: false,
          version: headerInfo.versionHex,
          compatible: false,
          reason: `Version ${headerInfo.versionHex} is not supported (expected: 0x200 or 0x300)`,
          fileTable: { ok: false, reason: "Skipped (unsupported version)" },
          pathEncoding: { ok: false, encoding: "unknown", reason: "Skipped (unsupported version)" },
        };
      }

      // Validate compacted file table + path encoding in one go
      const pathEncoding = this._analyzeGrfPathEncoding(fd, headerInfo, fileSize);

      const fileTableOk = pathEncoding.ok === true;
      const fileTable = fileTableOk
        ? { ok: true, layout: pathEncoding.layout, ...pathEncoding.table }
        : { ok: false, reason: pathEncoding.reason };

      if (!fileTableOk) {
        // If we can't inflate/parse the file table, it's not usable.
        return {
          valid: false,
          version: headerInfo.versionHex,
          compatible: false,
          reason: "Failed to read/parse compacted file table",
          fileTable,
          pathEncoding,
        };
      }

      // REAL TEST: try to load using the library (compatibility with runtime)
      testFd = fs.openSync(grfPath, "r");
      const grf = new GrfNode(testFd);

      try {
        const loadPromise = grf.load();
        const timeoutPromise = new Promise((_, reject) =>
          setTimeout(() => reject(new Error("GRF load timeout")), 10000)
        );

        await Promise.race([loadPromise, timeoutPromise]);

        fs.closeSync(testFd);
        testFd = null;

        fs.closeSync(fd);
        fd = null;

        return {
          valid: true,
          version: headerInfo.versionHex,
          compatible: true,
          reason: "GRF loaded successfully by the library",
          fileTable,
          pathEncoding,
        };
      } catch (loadError) {
        if (testFd) {
          try {
            fs.closeSync(testFd);
          } catch {}
          testFd = null;
        }
        if (fd) {
          try {
            fs.closeSync(fd);
          } catch {}
          fd = null;
        }

        return {
          valid: false,
          version: headerInfo.versionHex,
          compatible: false,
          reason: `Library failed to load: ${loadError.message}`,
          fileTable,
          pathEncoding,
        };
      }
    } catch (error) {
      if (testFd) {
        try {
          fs.closeSync(testFd);
        } catch {}
      }
      if (fd) {
        try {
          fs.closeSync(fd);
        } catch {}
      }

      return {
        valid: false,
        version: "error",
        compatible: false,
        reason: `Failed to validate GRF: ${error.message}`,
        fileTable: { ok: false, reason: "Exception" },
        pathEncoding: { ok: false, encoding: "unknown", reason: "Exception" },
      };
    }
  }

  /**
   * Deep encoding validation using @chicowall/grf-loader
   * Validates ALL files in GRFs and returns detailed encoding statistics
   */
  async validateEncodingDeep(grfFiles) {
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

    const resourcesPath = path.join(process.cwd(), "resources");

    const results = {
      iconvAvailable,
      grfs: [],
      summary: {
        totalFiles: 0,
        badUfffd: 0,
        badC1Control: 0,
        mojibakeDetected: 0,
        needsConversion: 0,
        healthPercent: 100,
      },
      filesToConvert: [],
    };

    // Helper: check for C1 controls (U+0080..U+009F)
    const hasC1Controls = (s) => {
      for (let i = 0; i < s.length; i++) {
        const c = s.charCodeAt(i);
        if (c >= 0x80 && c <= 0x9f) return true;
      }
      return false;
    };

    for (const grfFile of grfFiles) {
      const grfPath = path.join(resourcesPath, grfFile);
      if (!fs.existsSync(grfPath)) continue;

      let fd = null;
      const grfResult = {
        file: grfFile,
        totalFiles: 0,
        badUfffd: 0,
        badC1Control: 0,
        mojibakeDetected: 0,
        examples: {
          badUfffd: [],
          badC1Control: [],
          mojibake: [],
        },
        detectedEncoding: null,
      };

      try {
        fd = fs.openSync(grfPath, "r");
        const grf = new GrfNode(fd, { filenameEncoding: "auto" });
        await grf.load();

        const stats = grf.getStats?.() ?? {};
        grfResult.totalFiles = stats.fileCount || 0;
        grfResult.detectedEncoding = stats.detectedEncoding || "unknown";

        // Iterate ALL files
        const allFiles = grf.files ? Array.from(grf.files.keys()) : [];

        for (const filename of allFiles) {
          const s = String(filename);

          // Check U+FFFD
          if (s.includes("\uFFFD")) {
            grfResult.badUfffd++;
            if (grfResult.examples.badUfffd.length < 10) {
              grfResult.examples.badUfffd.push(s);
            }
          }

          // Check C1 controls
          if (hasC1Controls(s)) {
            grfResult.badC1Control++;
            if (grfResult.examples.badC1Control.length < 10) {
              grfResult.examples.badC1Control.push(s);
            }
          }

          // Check mojibake
          if (isMojibake(s)) {
            grfResult.mojibakeDetected++;
            if (grfResult.examples.mojibake.length < 10) {
              const koreanPath = fixMojibake(s);
              // grfPath = path in GRF (mojibake), koreanPath = correct Korean path
              grfResult.examples.mojibake.push({ grfPath: s, koreanPath });
            }
          }
        }

        // Update summary
        results.summary.totalFiles += grfResult.totalFiles;
        results.summary.badUfffd += grfResult.badUfffd;
        results.summary.badC1Control += grfResult.badC1Control;
        results.summary.mojibakeDetected += grfResult.mojibakeDetected;

        // Files needing conversion (mojibake or C1)
        const needsConv = grfResult.mojibakeDetected + grfResult.badC1Control;
        results.summary.needsConversion += needsConv;

        // Add examples to global list (koreanPath → grfPath mapping)
        grfResult.examples.mojibake.forEach((ex) => {
          if (results.filesToConvert.length < 50) {
            results.filesToConvert.push({ grf: grfFile, ...ex });
          }
        });

        results.grfs.push(grfResult);
      } catch (e) {
        grfResult.error = e.message;
        results.grfs.push(grfResult);
      } finally {
        if (fd !== null) {
          try {
            fs.closeSync(fd);
          } catch {}
        }
      }
    }

    // Calculate health percentage
    if (results.summary.totalFiles > 0) {
      const badCount = results.summary.badUfffd + results.summary.badC1Control;
      results.summary.healthPercent = parseFloat(
        (((results.summary.totalFiles - badCount) / results.summary.totalFiles) * 100).toFixed(4)
      );
    }

    this.validationResults.encoding = results;
    return results;
  }

  validateRequiredFiles() {
    const checks = [
      { path: "resources", type: "dir", required: true, name: "resources/ folder" },
      { path: "resources/DATA.INI", type: "file", required: true, name: "DATA.INI file" },
      { path: "BGM", type: "dir", required: false, name: "BGM/ folder" },
      { path: "data", type: "dir", required: false, name: "data/ folder" },
      { path: "System", type: "dir", required: false, name: "System/ folder" },
    ];

    let hasErrors = false;
    const results = [];

    for (const check of checks) {
      const fullPath = path.join(process.cwd(), check.path);
      const exists = fs.existsSync(fullPath);

      if (check.type === "dir") {
        const isEmpty = exists
          ? fs.readdirSync(fullPath).filter((f) => !f.startsWith("add-")).length === 0
          : true;

        results.push({ ...check, exists, isEmpty });

        if (check.required && !exists) {
          this.addError(`${check.name} not found!`);
          hasErrors = true;
        } else if (check.required && isEmpty) {
          this.addWarning(`${check.name} is empty`);
        } else if (!check.required && isEmpty) {
          this.addWarning(`${check.name} is empty - may cause issues depending on the client`);
        } else {
          this.addInfo(`${check.name} OK`);
        }
      } else {
        results.push({ ...check, exists });

        if (check.required && !exists) {
          this.addError(`${check.name} not found!`);
          hasErrors = true;
        } else if (exists) {
          this.addInfo(`${check.name} OK`);
        }
      }
    }

    this.validationResults.files = { valid: !hasErrors, checks: results };
    return !hasErrors;
  }

  validateEnvironment() {
    const envVars = {
      PORT: { value: process.env.PORT, default: "3338", required: false },
      CLIENT_PUBLIC_URL: { value: process.env.CLIENT_PUBLIC_URL, default: null, required: true },
      NODE_ENV: { value: process.env.NODE_ENV, default: "development", required: false },
    };

    let hasErrors = false;
    const results = {};

    if (!envVars.PORT.value) {
      this.addWarning(`PORT not set, using default: ${envVars.PORT.default}`);
      results.PORT = { defined: false, usingDefault: true, value: envVars.PORT.default };
    } else {
      this.addInfo(`PORT: ${envVars.PORT.value}`);
      results.PORT = { defined: true, value: envVars.PORT.value };
    }

    if (!envVars.CLIENT_PUBLIC_URL.value) {
      this.addError("CLIENT_PUBLIC_URL not set! Configure it in the .env file");
      hasErrors = true;
      results.CLIENT_PUBLIC_URL = { defined: false, error: "Variable not set" };
    } else {
      try {
        new URL(envVars.CLIENT_PUBLIC_URL.value);
        this.addInfo(`CLIENT_PUBLIC_URL: ${envVars.CLIENT_PUBLIC_URL.value}`);
        results.CLIENT_PUBLIC_URL = { defined: true, value: envVars.CLIENT_PUBLIC_URL.value };
      } catch {
        this.addError(`Invalid CLIENT_PUBLIC_URL: ${envVars.CLIENT_PUBLIC_URL.value}`);
        hasErrors = true;
        results.CLIENT_PUBLIC_URL = {
          defined: true,
          invalid: true,
          value: envVars.CLIENT_PUBLIC_URL.value,
        };
      }
    }

    const nodeEnv = envVars.NODE_ENV.value || envVars.NODE_ENV.default;
    results.NODE_ENV = { defined: !!envVars.NODE_ENV.value, value: nodeEnv };

    if (nodeEnv === "production") {
      const configs = require("../config/configs");
      if (configs.DEBUG) this.addWarning("DEBUG is enabled in PRODUCTION!");
    }

    if (!envVars.NODE_ENV.value) {
      this.addWarning(`NODE_ENV not set, using: ${nodeEnv}`);
    } else {
      this.addInfo(`NODE_ENV: ${nodeEnv}`);
    }

    const envPath = path.join(process.cwd(), ".env");
    const envExamplePath = path.join(process.cwd(), ".env.example");
    if (!fs.existsSync(envPath) && fs.existsSync(envExamplePath)) {
      this.addWarning(".env file not found! Copy .env.example to .env and configure it");
    }

    // Validate WS_ALLOWED_TARGETS when the proxy is enabled
    const wsProxyEnabled = process.env.ENABLE_WSPROXY === 'true';
    const wsAllowedRaw = process.env.WS_ALLOWED_TARGETS || '';
    if (wsProxyEnabled) {
      if (wsAllowedRaw) {
        const entries = wsAllowedRaw.split(',').map(s => s.trim()).filter(Boolean);
        const invalid = entries.filter(entry => {
          const colonIdx = entry.lastIndexOf(':');
          if (colonIdx === -1) return true;
          const port = parseInt(entry.slice(colonIdx + 1), 10);
          const host = entry.slice(0, colonIdx);
          return !host || !Number.isInteger(port) || port < 1 || port > 65535;
        });
        if (invalid.length > 0) {
          this.addError(
            `WS_ALLOWED_TARGETS contains invalid entries: ${invalid.join(', ')}\n` +
            `  Each entry must be "host:port" where port is 1-65535.`
          );
          hasErrors = true;
        } else {
          this.addInfo(`WS_ALLOWED_TARGETS: ${entries.join(', ')}`);
        }
        results.WS_ALLOWED_TARGETS = { defined: true, entries };
      } else {
        this.addInfo('WS_ALLOWED_TARGETS: not set, using localhost defaults (127.0.0.1:6900/6121/5121)');
        results.WS_ALLOWED_TARGETS = { defined: false, usingDefaults: true };
      }
    }

    this.validationResults.env = { valid: !hasErrors, variables: results };
    return !hasErrors;
  }

  async validateAll(options = {}) {
    const { deepEncoding = false } = options;

    console.log("\n🔍 Validating startup configuration...\n");

    this.validateNodeVersion();

    const depsValid = this.validateDependencies();
    if (!depsValid) return this.getResults();

    this.validateRequiredFiles();
    this.validateEnvironment();
    await this.validateGrfs();

    // Deep encoding validation (optional, slower)
    if (deepEncoding && this.validationResults.grfs?.files) {
      const grfFiles = this.validationResults.grfs.files
        .filter((g) => g.exists && g.valid)
        .map((g) => g.file);

      if (grfFiles.length > 0) {
        console.log("🔍 Running deep encoding validation...\n");
        const encodingResults = await this.validateEncodingDeep(grfFiles);

        // Add encoding warnings
        if (encodingResults.summary.mojibakeDetected > 0) {
          this.addWarning(
            `Mojibake detected: ${encodingResults.summary.mojibakeDetected} files need encoding conversion`
          );
        }
        if (encodingResults.summary.badUfffd > 0) {
          this.addWarning(
            `U+FFFD characters: ${encodingResults.summary.badUfffd} files have replacement characters`
          );
        }
        if (encodingResults.summary.badC1Control > 0) {
          this.addWarning(
            `C1 Control chars: ${encodingResults.summary.badC1Control} files have C1 control characters`
          );
        }

        this.addInfo(
          `Encoding health: ${encodingResults.summary.healthPercent}% (${encodingResults.summary.totalFiles} files)`
        );
      }
    }

    return this.getResults();
  }

  getResults() {
    return {
      success: this.errors.length === 0,
      errors: this.errors,
      warnings: this.warnings,
      info: this.info,
      details: this.validationResults,
    };
  }

  _printMultiline(prefix, text) {
    for (const line of String(text).split("\n")) console.log(prefix + line);
  }

  printReport(results = null) {
    if (!results) results = this.getResults();

    console.log("\n" + "=".repeat(80));
    console.log("📋 VALIDATION REPORT");
    console.log("=".repeat(80) + "\n");

    if (results.info.length > 0) {
      console.log("✓ INFO:");
      results.info.forEach((msg) => this._printMultiline("  ", msg));
      console.log("");
    }

    if (results.warnings.length > 0) {
      console.log("⚠️  WARNINGS:");
      results.warnings.forEach((msg) => this._printMultiline("  ", msg));
      console.log("");
    }

    if (results.errors.length > 0) {
      console.log("❌ ERRORS:");
      results.errors.forEach((msg) => this._printMultiline("  ", msg));
      console.log("");
    }

    console.log("=".repeat(80));
    if (results.success) {
      console.log("✅ Validation completed successfully!");
      if (results.warnings.length > 0) console.log(`⚠️  ${results.warnings.length} warning(s) found`);
    } else {
      console.log("❌ Validation failed!");
      console.log(`   ${results.errors.length} error(s) found`);
      console.log('\n💡 Tip: Run "npm run doctor" for a detailed diagnosis');
    }
    console.log("=".repeat(80) + "\n");

    return results.success;
  }

  getStatusJSON() {
    const results = this.getResults();
    return {
      timestamp: new Date().toISOString(),
      status: results.success ? "ok" : "error",
      hasWarnings: results.warnings.length > 0,
      summary: {
        errors: results.errors.length,
        warnings: results.warnings.length,
        info: results.info.length,
      },
      details: results.details,
      messages: {
        errors: results.errors,
        warnings: results.warnings,
        info: results.info,
      },
    };
  }
}

module.exports = StartupValidator;
