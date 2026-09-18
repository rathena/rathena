#!/usr/bin/env node
/**
 * validate-grf.mjs
 *
 * Validates:
 *  1) Index integrity (counts, bad names, collisions)
 *  2) Path normalization (slash + case) via resolvePath()
 *  3) Optional: extraction/decompression for ALL files (heavy) or a sample
 *
 * Usage:
 *  node tools/validate-grf.mjs <grfPath> [encoding=auto] [mode=lookup|extract-all|extract-sample] [sampleN=200]
 *
 * Examples:
 *  node tools/validate-grf.mjs .\\resources\\data.grf auto lookup
 *  node tools/validate-grf.mjs .\\resources\\data.grf auto extract-sample 500
 *  node tools/validate-grf.mjs .\\resources\\data.grf auto extract-all
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
import { openSync, closeSync, writeFileSync } from "fs";
import path from "path";

const grfPath = process.argv[2];
const encoding = process.argv[3] || "auto";
const mode = process.argv[4] || "lookup";
const sampleN = Number(process.argv[5] || 200);

if (!grfPath) {
  console.error("Uso: node tools/validate-grf.mjs <grfPath> [encoding=auto] [mode=lookup|extract-all|extract-sample] [sampleN=200]");
  process.exit(1);
}

function norm(p) {
  return String(p).replace(/[\\/]+/g, "/").toLowerCase().normalize("NFC");
}

function nowStamp() {
  const d = new Date();
  const pad = (n) => String(n).padStart(2, "0");
  return `${d.getFullYear()}${pad(d.getMonth() + 1)}${pad(d.getDate())}-${pad(d.getHours())}${pad(d.getMinutes())}${pad(d.getSeconds())}`;
}

function sampleUnique(arr, k) {
  if (arr.length <= k) return arr.slice();
  const out = [];
  const used = new Set();
  while (out.length < k) {
    const i = Math.floor(Math.random() * arr.length);
    if (used.has(i)) continue;
    used.add(i);
    out.push(arr[i]);
  }
  return out;
}

function mkVariants(originalKey) {
  // user-input variants (slash/case)
  const v1 = originalKey.replace(/[\\]+/g, "/"); // slash variant
  const v2 = originalKey.toUpperCase();           // case variant (ASCII parts)
  const v3 = v1.toUpperCase();                    // slash+case variant
  return [v1, v2, v3];
}

function logProgress(i, total, extra = "") {
  const pct = ((i / total) * 100).toFixed(2);
  process.stdout.write(`\r[PROGRESS] ${i}/${total} (${pct}%) ${extra}`.padEnd(100, " "));
}

const fd = openSync(grfPath, "r");

const report = {
  input: {
    grfPath: path.resolve(grfPath),
    encodingRequested: encoding,
    mode,
    sampleN,
    startedAt: new Date().toISOString(),
  },
  stats: null,
  index: {
    fileCount: 0,
    badNameCount: 0,
    collisionCount: 0,
    collisionsSample: [],
    badNamesSample: [],
  },
  validation: {
    resolveChecks: {
      totalFilesChecked: 0,
      variantFailures: { slash: 0, upper: 0, slashUpper: 0 },
      ambiguous: 0,
      notFound: 0,
      examples: [],
    },
    extraction: {
      enabled: mode !== "lookup",
      mode,
      totalAttempted: 0,
      ok: 0,
      failed: 0,
      examples: [],
    },
  },
};

try {
  console.log(`[1] Loading GRF: ${grfPath}`);
  const grf = new GrfNode(fd, { filenameEncoding: encoding });
  await grf.load();

  const stats = grf.getStats?.() ?? {};
  report.stats = stats;

  const keys = grf.files ? Array.from(grf.files.keys()) : [];
  report.index.fileCount = keys.length;

  // bad names sample (U+FFFD)
  const bad = keys.filter((k) => String(k).includes("\uFFFD"));
  report.index.badNameCount = bad.length;
  report.index.badNamesSample = bad.slice(0, 20);

  // collision check (by normalized key)
  const buckets = new Map();
  for (const k of keys) {
    const nk = norm(k);
    const arr = buckets.get(nk) || [];
    arr.push(k);
    buckets.set(nk, arr);
  }
  const collisions = [];
  for (const [nk, arr] of buckets.entries()) {
    if (arr.length > 1) collisions.push({ norm: nk, count: arr.length, sample: arr.slice(0, 5) });
  }
  report.index.collisionCount = collisions.length;
  report.index.collisionsSample = collisions.slice(0, 20);

  console.log("[2] Stats:", stats);
  console.log(`[3] Index: fileCount=${report.index.fileCount} badNameCount=${report.index.badNameCount} collisionCount=${report.index.collisionCount}`);

  // resolvePath validation (ALL files)
  if (!grf.resolvePath) {
    console.log("[WARN] grf.resolvePath() não existe. Pulando validação de normalização.");
  } else {
    console.log("[4] Validando normalização (resolvePath) em TODOS os arquivos...");
    const total = keys.length;
    let i = 0;

    for (const original of keys) {
      i++;
      if (i % 2000 === 0 || i === total) logProgress(i, total);

      const variants = mkVariants(original);
      const labels = ["slash", "upper", "slashUpper"];

      for (let j = 0; j < variants.length; j++) {
        const q = variants[j];
        const label = labels[j];

        const res = grf.resolvePath(q);
        if (!res || res.status !== "found") {
          report.validation.resolveChecks.variantFailures[label]++;

          if (res?.status === "ambiguous") report.validation.resolveChecks.ambiguous++;
          else if (res?.status === "not_found") report.validation.resolveChecks.notFound++;

          if (report.validation.resolveChecks.examples.length < 50) {
            report.validation.resolveChecks.examples.push({ original, variantLabel: label, query: q, result: res });
          }
        } else {
          const okSame = res.matchedPath === original || norm(res.matchedPath) === norm(original);
          if (!okSame) {
            report.validation.resolveChecks.variantFailures[label]++;
            if (report.validation.resolveChecks.examples.length < 50) {
              report.validation.resolveChecks.examples.push({
                original,
                variantLabel: label,
                query: q,
                result: res,
                note: "found, but matchedPath differs (norm mismatch?)",
              });
            }
          }
        }
      }

      report.validation.resolveChecks.totalFilesChecked = i;
    }
    process.stdout.write("\n");
    console.log("[5] OK: resolvePath validation finished.");
    console.log("    Failures:", report.validation.resolveChecks.variantFailures);
  }

  // extraction validation
  if (mode === "extract-all" || mode === "extract-sample") {
    console.log(`[6] Extração: mode=${mode} (extract-all é pesado)`);
    const list = mode === "extract-all" ? keys : sampleUnique(keys, Math.max(1, sampleN));

    const total = list.length;
    let i = 0;
    for (const k of list) {
      i++;
      if (i % 400 === 0 || i === total) logProgress(i, total, "extract");

      report.validation.extraction.totalAttempted++;
      try {
        const { data, error } = await grf.getFile(k);
        if (error) {
          report.validation.extraction.failed++;
          if (report.validation.extraction.examples.length < 50) report.validation.extraction.examples.push({ key: k, error: String(error) });
        } else {
          // allow 0 bytes, but data must have length
          if (!data || typeof data.length !== "number") {
            report.validation.extraction.failed++;
            if (report.validation.extraction.examples.length < 50) report.validation.extraction.examples.push({ key: k, error: "data inválido (sem length)" });
          } else {
            report.validation.extraction.ok++;
          }
        }
      } catch (e) {
        report.validation.extraction.failed++;
        if (report.validation.extraction.examples.length < 50) report.validation.extraction.examples.push({ key: k, error: String(e?.message || e) });
      }
    }
    process.stdout.write("\n");
    console.log("[7] OK: extraction validation finished.");
    console.log(`    attempted=${report.validation.extraction.totalAttempted} ok=${report.validation.extraction.ok} failed=${report.validation.extraction.failed}`);
  }

  // write report
  report.input.finishedAt = new Date().toISOString();

  const outName = `validate-grf-report-${path.basename(grfPath).replace(/[^a-zA-Z0-9._-]+/g, "_")}-${nowStamp()}.json`;
  const outPath = path.join(process.cwd(), outName);
  writeFileSync(outPath, JSON.stringify(report, null, 2), "utf-8");
  console.log(`[8] Report saved: ${outPath}`);

  const resolveFail =
    report.validation.resolveChecks.variantFailures.slash +
      report.validation.resolveChecks.variantFailures.upper +
      report.validation.resolveChecks.variantFailures.slashUpper;

  const extractionFail = report.validation.extraction.enabled ? report.validation.extraction.failed : 0;

  if (report.index.collisionCount > 0 || report.index.badNameCount > 0 || resolveFail > 0 || extractionFail > 0) {
    console.log("[RESULT] ⚠️  Tem alertas/falhas. Veja o report JSON (examples).");
    process.exit(2);
  } else {
    console.log("[RESULT] ✅ Tudo OK.");
    process.exit(0);
  }
} finally {
  closeSync(fd);
}
