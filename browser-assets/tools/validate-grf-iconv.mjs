#!/usr/bin/env node
/**
 * validate-grf-iconv.mjs
 *
 * Valida TODOS os nomes (pastas+arquivos):
 *  - Detecta U+FFFD "�"
 *  - Round-trip: str -> encode(enc) -> decode(enc) deve voltar igual
 *  - Mojibake check: str -> encode(enc) -> latin1 string -> decode(enc) deve voltar igual
 *
 * Uso:
 *  node tools/validate-grf-iconv.mjs <grfPath> [encoding=auto]
 *
 * Ex:
 *  node tools/validate-grf-iconv.mjs .\\resources\\data.grf auto
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
import iconv from "iconv-lite";

const grfPath = process.argv[2];
const encodingRequested = process.argv[3] || "auto";

if (!grfPath) {
  console.error("Uso: node tools/validate-grf-iconv.mjs <grfPath> [encoding=auto]");
  process.exit(1);
}

function norm(p) {
  return String(p).replace(/[\\/]+/g, "/").toLowerCase().normalize("NFC");
}

function mapToObj(m) {
  if (!m || typeof m.entries !== "function") return m;
  return Object.fromEntries(m.entries());
}

function stamp() {
  const d = new Date();
  const pad = (n) => String(n).padStart(2, "0");
  return `${d.getFullYear()}${pad(d.getMonth() + 1)}${pad(d.getDate())}-${pad(d.getHours())}${pad(d.getMinutes())}${pad(d.getSeconds())}`;
}

function logProgress(i, total) {
  const pct = total ? ((i / total) * 100).toFixed(2) : "??";
  process.stdout.write(`\r[PROGRESS] ${i}/${total} (${pct}%)`.padEnd(80, " "));
}

const fd = openSync(grfPath, "r");

const report = {
  input: {
    grfPath: path.resolve(grfPath),
    encodingRequested,
    startedAt: new Date().toISOString(),
  },
  stats: null,
  encodingUsed: null,
  summary: {
    total: 0,
    badUfffd: 0,
    roundTripFail: 0,
    mojibakeFail: 0,
    segmentRoundTripFail: 0,
    encodeError: 0,
  },
  examples: {
    badUfffd: [],
    roundTripFail: [],
    mojibakeFail: [],
    segmentRoundTripFail: [],
    encodeError: [],
  },
};

try {
  console.log(`[1] Loading GRF: ${grfPath}`);
  const grf = new GrfNode(fd, { filenameEncoding: encodingRequested });
  await grf.load();

  
const result = await grf.getFile("data\\texture\\유저인터페이스\\cardbmp\\pp카드.bmp");
console.log(result.data ? "✅ Arquivo lido!" : "❌ Falhou");

  const stats = grf.getStats?.() ?? {};
  // Map não serializa em JSON
  if (stats.extensionStats) stats.extensionStats = mapToObj(stats.extensionStats);

  report.stats = stats;

  const encodingUsed = stats.detectedEncoding || (encodingRequested !== "auto" ? encodingRequested : "euc-kr");
  report.encodingUsed = encodingUsed;

  const total = stats.fileCount ?? (grf.files ? grf.files.size : 0);
  console.log(`[2] detectedEncoding=${encodingUsed} total=${total}`);

  let i = 0;
  for (const key of grf.files.keys()) {
    i++;
    if (i % 2000 === 0 || i === total) logProgress(i, total);

    report.summary.total++;

    const s = String(key);

    // 1) U+FFFD check
    if (s.includes("\uFFFD")) {
      report.summary.badUfffd++;
      if (report.examples.badUfffd.length < 50) report.examples.badUfffd.push(s);
    }

    // 2) Round-trip do path inteiro
    try {
      const buf = iconv.encode(s, encodingUsed);
      const back = iconv.decode(buf, encodingUsed);
      if (back !== s) {
        report.summary.roundTripFail++;
        if (report.examples.roundTripFail.length < 50) {
          report.examples.roundTripFail.push({
            key: s,
            back,
          });
        }
      }

      // 3) Mojibake: encode(enc) -> latin1 string -> decode(enc)
      const moj = buf.toString("latin1");
      const back2 = iconv.decode(Buffer.from(moj, "latin1"), encodingUsed);
      if (back2 !== s) {
        report.summary.mojibakeFail++;
        if (report.examples.mojibakeFail.length < 50) {
          report.examples.mojibakeFail.push({
            key: s,
            mojibake: moj,
            back: back2,
          });
        }
      }

      // 4) Round-trip por segmento (pasta/arquivo), pega bugs “localizados”
      const segs = s.split(/[\\/]+/g);
      let segFail = false;
      for (const seg of segs) {
        if (!seg) continue;
        const b = iconv.encode(seg, encodingUsed);
        const bb = iconv.decode(b, encodingUsed);
        if (bb !== seg) {
          segFail = true;
          if (report.examples.segmentRoundTripFail.length < 50) {
            report.examples.segmentRoundTripFail.push({
              key: s,
              segment: seg,
              segmentBack: bb,
            });
          }
          break;
        }
      }
      if (segFail) report.summary.segmentRoundTripFail++;
    } catch (e) {
      report.summary.encodeError++;
      if (report.examples.encodeError.length < 50) {
        report.examples.encodeError.push({ key: s, error: String(e?.message || e) });
      }
    }
  }

  process.stdout.write("\n");

  report.input.finishedAt = new Date().toISOString();

  const outName = `validate-grf-iconv-${path.basename(grfPath).replace(/[^a-zA-Z0-9._-]+/g, "_")}-${stamp()}.json`;
  const outPath = path.join(process.cwd(), outName);
  writeFileSync(outPath, JSON.stringify(report, null, 2), "utf-8");

  console.log("[3] Summary:", report.summary);
  console.log(`[4] Report saved: ${outPath}`);

  // Fail só se tiver problema real de encoding (além de U+FFFD)
  const hardFail =
    report.summary.roundTripFail > 0 ||
    report.summary.mojibakeFail > 0 ||
    report.summary.segmentRoundTripFail > 0 ||
    report.summary.encodeError > 0;

  if (hardFail) {
    console.log("[RESULT] ❌ Falhou (problema real de encoding/representabilidade). Veja o report.");
    process.exit(2);
  } else if (report.summary.badUfffd > 0) {
    console.log("[RESULT] ⚠️  OK com warning: ainda existe U+FFFD em alguns nomes. Veja examples.badUfffd.");
    process.exit(0);
  } else {
    console.log("[RESULT] ✅ OK total.");
    process.exit(0);
  }
} finally {
  closeSync(fd);
}
