#!/usr/bin/env node
/**
 * validate-all-grfs.mjs (improved)
 *
 * Valida TODOS os arquivos GRF em uma pasta:
 *  - Detecta nomes com U+FFFD e C1 controls (U+0080-U+009F)
 *  - Faz round-trip de encoding (RAW e também após "reparos" heurísticos)
 *  - Testa leitura real de arquivos (amostragem melhor)
 *  - Gera relatório detalhado
 *
 * Uso:
 *  node tools/validate-all-grfs.mjs <pasta> [encoding=auto] [--read=100] [--examples=20]
 *
 * Ex:
 *  node tools/validate-all-grfs.mjs D:\\GRFs auto
 *  node tools/validate-all-grfs.mjs ./resources cp949 --read=300
 */

// Load the CommonJS build on purpose.
//
// The package's ESM build resolves iconv-lite through a bundler shim that needs a global `require`.
// Under `node -e` one exists and everything appears to work; inside a real .mjs there is none, the
// shim throws, iconv-lite is null, and isMojibake/fixMojibake/toMojibake silently become identity
// functions. That made `npm run convert:encoding` produce an empty mapping while reporting success,
// and made the validators flag encoding errors in archives the server reads perfectly.
import { createRequire } from "module";
const { GrfNode } = createRequire(import.meta.url)("@chicowall/grf-loader");
import { openSync, closeSync, writeFileSync, readdirSync, statSync } from "fs";
import path from "path";
import iconv from "iconv-lite";

// ============================================================================
// CLI / Configuration
// ============================================================================

const grfFolder = process.argv[2];
const encodingRequested = (process.argv[3] || "auto").toLowerCase();

const argRead = process.argv.find((a) => a.startsWith("--read="));
const argExamples = process.argv.find((a) => a.startsWith("--examples="));

const MAX_READ_TESTS = argRead ? Math.max(0, parseInt(argRead.split("=")[1], 10) || 0) : 100;
const MAX_EXAMPLES = argExamples ? Math.max(1, parseInt(argExamples.split("=")[1], 10) || 0) : 20;

if (!grfFolder) {
  console.error("Uso: node tools/validate-all-grfs.mjs <pasta> [encoding=auto] [--read=100] [--examples=20]");
  console.error("Exemplos:");
  console.error("  node tools/validate-all-grfs.mjs D:\\\\GRFs auto");
  console.error("  node tools/validate-all-grfs.mjs ./resources cp949 --read=300");
  process.exit(1);
}

// ============================================================================
// Helpers
// ============================================================================

function findGrfFiles(folder) {
  const grfFiles = [];
  function scan(dir) {
    try {
      const entries = readdirSync(dir);
      for (const entry of entries) {
        const fullPath = path.join(dir, entry);
        try {
          const stat = statSync(fullPath);
          if (stat.isDirectory()) scan(fullPath);
          else if (entry.toLowerCase().endsWith(".grf")) grfFiles.push(fullPath);
        } catch {}
      }
    } catch {}
  }
  scan(folder);
  return grfFiles;
}

function formatBytes(bytes) {
  if (bytes < 1024) return bytes + " B";
  if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + " KB";
  if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(1) + " MB";
  return (bytes / (1024 * 1024 * 1024)).toFixed(2) + " GB";
}

function stamp() {
  const d = new Date();
  const pad = (n) => String(n).padStart(2, "0");
  return `${d.getFullYear()}${pad(d.getMonth() + 1)}${pad(d.getDate())}-${pad(d.getHours())}${pad(d.getMinutes())}${pad(d.getSeconds())}`;
}

function mapToObj(m) {
  if (!m || typeof m.entries !== "function") return m;
  return Object.fromEntries(m.entries());
}

function logProgress(current, total, message) {
  const pct = total ? ((current / total) * 100).toFixed(1) : "??";
  process.stdout.write(`\r[${pct}%] ${message}`.padEnd(100, " "));
}

function hasC1Controls(s) {
  // U+0080..U+009F
  for (let i = 0; i < s.length; i++) {
    const c = s.charCodeAt(i);
    if (c >= 0x80 && c <= 0x9f) return true;
  }
  return false;
}

function countC1Controls(s) {
  let n = 0;
  for (let i = 0; i < s.length; i++) {
    const c = s.charCodeAt(i);
    if (c >= 0x80 && c <= 0x9f) n++;
  }
  return n;
}

function countHangul(s) {
  // Hangul Syllables block
  const m = s.match(/[\uAC00-\uD7A3]/g);
  return m ? m.length : 0;
}

function countReplacement(s) {
  const m = s.match(/\uFFFD/g);
  return m ? m.length : 0;
}

function normalizeEncodingForValidation(enc) {
  const e = (enc || "").toLowerCase();

  // Para validação em Node, cp949 cobre EUC-KR + extensões Windows-949/UHC.
  // Se vier "euc-kr" do auto, valida com cp949 pra não perder extensões.
  if (
    e === "cp949" ||
    e === "ms949" ||
    e === "uhc" ||
    e === "windows-949" ||
    e === "euc-kr" ||
    e === "cseuckr" ||
    e === "ks_c_5601-1987"
  ) {
    return "cp949";
  }

  if (e === "auto") return "cp949";
  return e || "cp949";
}

function safeIconvEncode(str, enc) {
  const e = normalizeEncodingForValidation(enc);
  if (!iconv.encodingExists(e)) return iconv.encode(str, "utf-8");
  return iconv.encode(str, e);
}

function safeIconvDecode(buf, enc) {
  const e = normalizeEncodingForValidation(enc);
  if (!iconv.encodingExists(e)) return iconv.decode(buf, "utf-8");
  return iconv.decode(buf, e);
}

/**
 * Heurística 1: "mojibake" cp949->latin1 (ex: "ºê¶óµð¿ò...")
 * Se converter latin1->cp949 melhora (mais Hangul, menos C1/FFFD), aplica.
 */
function maybeFixLatin1Mojibake(s, enc) {
  // Se não tem nada "alto", não perde tempo
  if (!/[\u00A0-\u00FF]/.test(s)) return s;

  const before = {
    hangul: countHangul(s),
    c1: countC1Controls(s),
    rep: countReplacement(s),
  };

  // Trata string como bytes 0x00..0xFF
  const bytes = Buffer.from(s, "latin1");
  const fixed = safeIconvDecode(bytes, enc);

  const after = {
    hangul: countHangul(fixed),
    c1: countC1Controls(fixed),
    rep: countReplacement(fixed),
  };

  // Aplica se melhora claramente
  const improved =
    after.rep <= before.rep &&
    after.c1 <= before.c1 &&
    after.hangul >= before.hangul + 2; // ganho mínimo de Hangul

  return improved ? fixed : s;
}

/**
 * Heurística 2: C1 control prefix típico de CP949 estendido lido errado (ex: "pp카드")
 * Repara SOMENTE o prefixo que está em faixa 0x00..0xFF (latin1),
 * decodifica em cp949 e concatena com o resto (Hangul já “ok”).
 */
function fixC1PrefixInSegment(seg, enc) {
  if (!hasC1Controls(seg)) return seg;

  // pega prefixo de bytes (<=0xFF) até o primeiro char >0xFF
  const bytes = [];
  let i = 0;
  for (; i < seg.length; i++) {
    const code = seg.charCodeAt(i);
    if (code <= 0xff) bytes.push(code);
    else break;
  }
  if (!bytes.length) return seg;

  const decodedPrefix = safeIconvDecode(Buffer.from(bytes), enc);
  const merged = decodedPrefix + seg.slice(i);

  // aplica se remove C1 e não adiciona mais U+FFFD
  const beforeC1 = countC1Controls(seg);
  const afterC1 = countC1Controls(merged);

  const beforeRep = countReplacement(seg);
  const afterRep = countReplacement(merged);

  if (afterC1 < beforeC1 && afterRep <= beforeRep) return merged;
  return seg;
}

function repairFilenameForValidation(filename, enc) {
  let s = filename;

  // 1) tenta consertar mojibake no path inteiro
  s = maybeFixLatin1Mojibake(s, enc);

  // 2) tenta consertar C1 por segmento
  const parts = s.split(/\\|\//);
  const repairedParts = parts.map((p) => fixC1PrefixInSegment(p, enc));
  s = repairedParts.join("\\");

  return s;
}

function roundTripOk(str, enc) {
  try {
    const b = safeIconvEncode(str, enc);
    const back = safeIconvDecode(b, enc);
    return back === str;
  } catch {
    return false;
  }
}

/**
 * Amostragem melhor:
 * - pega 1/3 do começo
 * - 1/3 do meio
 * - 1/3 aleatório (único)
 * - e prioriza “suspeitos” (C1/FFFD) se existirem
 */
function pickReadTestFiles(allFiles, suspicious, maxN) {
  if (maxN <= 0) return [];

  const out = [];
  const wantSus = Math.min(Math.floor(maxN * 0.4), suspicious.length);
  for (let i = 0; i < wantSus; i++) out.push(suspicious[i]);

  const remaining = maxN - out.length;
  if (remaining <= 0) return Array.from(new Set(out)).slice(0, maxN);

  const third = Math.max(1, Math.floor(remaining / 3));

  // começo
  for (let i = 0; i < Math.min(third, allFiles.length); i++) out.push(allFiles[i]);

  // meio
  const midStart = Math.max(0, Math.floor(allFiles.length / 2) - Math.floor(third / 2));
  for (let i = 0; i < third && midStart + i < allFiles.length; i++) out.push(allFiles[midStart + i]);

  // aleatório
  while (out.length < maxN && out.length < allFiles.length) {
    const idx = Math.floor(Math.random() * allFiles.length);
    out.push(allFiles[idx]);
  }

  return Array.from(new Set(out)).slice(0, maxN);
}

// ============================================================================
// Validation
// ============================================================================

async function validateGrf(grfPath, encoding) {
  const result = {
    path: grfPath,
    filename: path.basename(grfPath),
    size: 0,
    sizeFormatted: "",
    encodingRequested: encoding,
    detectedEncoding: null,
    encodingValidate: null,
    loadTimeMs: 0,
    success: false,
    error: null,
    stats: null,

    validation: {
      totalFiles: 0,

      badUfffd: 0,
      badC1Control: 0,

      // Round-trip em cima do que o loader deu (RAW)
      roundTripFailRaw: 0,

      // Quantos RAW falharam mas passam após reparo heurístico (sinal forte de decode errado)
      roundTripRepairable: 0,

      // Quantos continuam falhando mesmo após reparo (provável lixo real / caracteres fora do encoding)
      roundTripFailFinal: 0,

      readTestsPassed: 0,
      readTestsFailed: 0,
    },

    examples: {
      badUfffd: [],
      badC1Control: [],
      roundTripFailRaw: [],
      roundTripRepairable: [],
      roundTripFailFinal: [],
      readFailed: [],
    },
  };

  let fd = null;

  try {
    const stat = statSync(grfPath);
    result.size = stat.size;
    result.sizeFormatted = formatBytes(stat.size);

    fd = openSync(grfPath, "r");
    const grf = new GrfNode(fd, { filenameEncoding: encoding });

    const t0 = Date.now();
    await grf.load();
    result.loadTimeMs = Date.now() - t0;

    const stats = grf.getStats?.() ?? {};
    if (stats.extensionStats) stats.extensionStats = mapToObj(stats.extensionStats);
    result.stats = stats;

    result.detectedEncoding = (stats.detectedEncoding || encoding || "auto").toLowerCase();
    result.encodingValidate = normalizeEncodingForValidation(result.detectedEncoding);

    // lista de arquivos (strings já decodificadas pelo loader)
    const allFiles = Array.from(grf.files.keys());
    result.validation.totalFiles = stats.fileCount || allFiles.length;

    // suspeitos para priorizar read test
    const suspicious = [];

    // Validate filenames (com progress)
    for (let i = 0; i < allFiles.length; i++) {
      const filename = allFiles[i];

      if (i % 5000 === 0) {
        logProgress(i, allFiles.length, `Validating names for ${result.filename}...`);
      }

      const hasUfffd = filename.includes("\uFFFD");
      const hasC1 = hasC1Controls(filename);

      if (hasUfffd) {
        result.validation.badUfffd++;
        suspicious.push(filename);
        if (result.examples.badUfffd.length < MAX_EXAMPLES) result.examples.badUfffd.push(filename);
      }

      if (hasC1) {
        result.validation.badC1Control++;
        suspicious.push(filename);
        if (result.examples.badC1Control.length < MAX_EXAMPLES) result.examples.badC1Control.push(filename);
      }

      const rawOk = roundTripOk(filename, result.encodingValidate);
      if (!rawOk) {
        result.validation.roundTripFailRaw++;
        if (result.examples.roundTripFailRaw.length < MAX_EXAMPLES) {
          const back = safeIconvDecode(safeIconvEncode(filename, result.encodingValidate), result.encodingValidate);
          result.examples.roundTripFailRaw.push({ original: filename, roundTrip: back });
        }
      }

      // tenta reparo: se passar após reparo, é sinal de decode errado (C1/mojibake)
      const repaired = repairFilenameForValidation(filename, result.encodingValidate);
      const repairedOk = roundTripOk(repaired, result.encodingValidate);

      if (!rawOk && repairedOk) {
        result.validation.roundTripRepairable++;
        if (result.examples.roundTripRepairable.length < MAX_EXAMPLES) {
          result.examples.roundTripRepairable.push({ original: filename, repaired });
        }
      }

      if (!repairedOk) {
        result.validation.roundTripFailFinal++;
        if (result.examples.roundTripFailFinal.length < MAX_EXAMPLES) {
          const back2 = safeIconvDecode(safeIconvEncode(repaired, result.encodingValidate), result.encodingValidate);
          result.examples.roundTripFailFinal.push({ original: filename, repaired, roundTrip: back2 });
        }
      }
    }

    process.stdout.write("\n");

    // Read tests (amostragem melhor + suspeitos)
    const filesToTest = pickReadTestFiles(allFiles, suspicious, MAX_READ_TESTS);

    for (let i = 0; i < filesToTest.length; i++) {
      const filename = filesToTest[i];
      if (i % 10 === 0) logProgress(i, filesToTest.length, `Testing reads for ${result.filename}...`);

      try {
        const fileResult = await grf.getFile(filename);
        if (fileResult?.data?.length > 0) {
          result.validation.readTestsPassed++;
        } else {
          result.validation.readTestsFailed++;
          if (result.examples.readFailed.length < MAX_EXAMPLES) {
            result.examples.readFailed.push({ filename, error: fileResult?.error || "empty/no data" });
          }
        }
      } catch (e) {
        result.validation.readTestsFailed++;
        if (result.examples.readFailed.length < MAX_EXAMPLES) {
          result.examples.readFailed.push({ filename, error: String(e?.message || e) });
        }
      }
    }

    process.stdout.write("\n");

    result.success = true;
  } catch (e) {
    result.success = false;
    result.error = String(e?.message || e);
  } finally {
    if (fd !== null) {
      try { closeSync(fd); } catch {}
    }
  }

  return result;
}

// ============================================================================
// Main
// ============================================================================

async function main() {
  console.log("=".repeat(80));
  console.log("GRF Validation Tool (Improved)");
  console.log("=".repeat(80));
  console.log(`Folder:   ${path.resolve(grfFolder)}`);
  console.log(`Encoding: ${encodingRequested}`);
  console.log(`ReadTests per GRF: ${MAX_READ_TESTS}`);
  console.log(`Examples per bucket: ${MAX_EXAMPLES}`);
  console.log("");

  console.log("[1] Scanning for GRF files...");
  const grfFiles = findGrfFiles(grfFolder);
  if (grfFiles.length === 0) {
    console.error("No GRF files found!");
    process.exit(1);
  }
  console.log(`    Found ${grfFiles.length} GRF file(s)\n`);

  const report = {
    meta: {
      folder: path.resolve(grfFolder),
      encodingRequested,
      startedAt: new Date().toISOString(),
      finishedAt: null,
      grfCount: grfFiles.length,
    },
    summary: {
      totalGrfs: grfFiles.length,
      successfulLoads: 0,
      failedLoads: 0,

      totalFiles: 0,

      totalBadUfffd: 0,
      totalBadC1Control: 0,

      totalRoundTripFailRaw: 0,
      totalRoundTripRepairable: 0,
      totalRoundTripFailFinal: 0,

      totalReadTestsPassed: 0,
      totalReadTestsFailed: 0,
    },
    grfs: [],
  };

  console.log("[2] Validating GRF files...\n");

  for (let i = 0; i < grfFiles.length; i++) {
    const grfPath = grfFiles[i];
    const grfName = path.basename(grfPath);

    console.log(`[${i + 1}/${grfFiles.length}] ${grfName}`);

    const result = await validateGrf(grfPath, encodingRequested);
    report.grfs.push(result);

    if (result.success) {
      report.summary.successfulLoads++;

      report.summary.totalFiles += result.validation.totalFiles;

      report.summary.totalBadUfffd += result.validation.badUfffd;
      report.summary.totalBadC1Control += result.validation.badC1Control;

      report.summary.totalRoundTripFailRaw += result.validation.roundTripFailRaw;
      report.summary.totalRoundTripRepairable += result.validation.roundTripRepairable;
      report.summary.totalRoundTripFailFinal += result.validation.roundTripFailFinal;

      report.summary.totalReadTestsPassed += result.validation.readTestsPassed;
      report.summary.totalReadTestsFailed += result.validation.readTestsFailed;

      console.log(
        `    ✅ Loaded: ${result.validation.totalFiles} files, ${result.loadTimeMs}ms, ` +
        `detected=${result.detectedEncoding}, validateAs=${result.encodingValidate}`
      );

      console.log(
        `       BadNames: U+FFFD=${result.validation.badUfffd}, ` +
        `C1=${result.validation.badC1Control}`
      );

      console.log(
        `       RoundTrip: rawFail=${result.validation.roundTripFailRaw}, ` +
        `repairable=${result.validation.roundTripRepairable}, ` +
        `finalFail=${result.validation.roundTripFailFinal}`
      );

      console.log(
        `       Read tests: ${result.validation.readTestsPassed} passed, ${result.validation.readTestsFailed} failed`
      );
    } else {
      report.summary.failedLoads++;
      console.log(`    ❌ Failed: ${result.error}`);
    }

    console.log("");
  }

  report.meta.finishedAt = new Date().toISOString();

  console.log("=".repeat(80));
  console.log("SUMMARY");
  console.log("=".repeat(80));
  console.log(`GRFs loaded:              ${report.summary.successfulLoads}/${report.summary.totalGrfs}`);
  console.log(`Total files:              ${report.summary.totalFiles.toLocaleString()}`);
  console.log(`Bad U+FFFD:               ${report.summary.totalBadUfffd.toLocaleString()}`);
  console.log(`Bad C1 Control:           ${report.summary.totalBadC1Control.toLocaleString()}`);
  console.log(`Round-trip fails (RAW):    ${report.summary.totalRoundTripFailRaw.toLocaleString()}`);
  console.log(`Round-trip repairable:     ${report.summary.totalRoundTripRepairable.toLocaleString()}`);
  console.log(`Round-trip fails (FINAL):  ${report.summary.totalRoundTripFailFinal.toLocaleString()}`);
  console.log(`Read tests passed:         ${report.summary.totalReadTestsPassed.toLocaleString()}`);
  console.log(`Read tests failed:         ${report.summary.totalReadTestsFailed.toLocaleString()}`);
  console.log("");

  // Health score (considera "clean" sem U+FFFD e sem C1)
  const totalBadNameSignals = report.summary.totalBadUfffd + report.summary.totalBadC1Control;
  const healthPct = report.summary.totalFiles > 0
    ? ((report.summary.totalFiles - totalBadNameSignals) / report.summary.totalFiles * 100).toFixed(4)
    : "0.0000";

  console.log(`Encoding Health (no U+FFFD/C1): ${healthPct}%`);

  const outName = `grf-validation-${stamp()}.json`;
  const outPath = path.join(process.cwd(), outName);
  writeFileSync(outPath, JSON.stringify(report, null, 2), "utf-8");
  console.log(`\nReport saved: ${outPath}`);

  // Exit code:
  // - 2 se falhou load
  // - 1 se tem "finalFail" (problema real/irreversível) OU read failed
  // - 0 caso contrário
  if (report.summary.failedLoads > 0) process.exit(2);
  if (report.summary.totalRoundTripFailFinal > 0 || report.summary.totalReadTestsFailed > 0) process.exit(1);
  process.exit(0);
}

main().catch((e) => {
  console.error("Fatal error:", e);
  process.exit(1);
});
