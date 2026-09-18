#!/usr/bin/env node
// extract-grf.mjs — standalone Ragnarok Online GRF/GPF extractor.
//
// Reads Gravity's GRF archive formats and writes the (decompressed, decrypted)
// files to disk. Handles GRF versions 0x101 / 0x103 / 0x200 AND the custom
// 0x300 "Event Horizon" fork used by recent official clients (e.g. ROLatam),
// including the per-entry custom DES encryption — which the standard zextractor
// tool cannot read.
//
// No dependencies beyond Node's stdlib (needs Node 18+ for the EUC-KR decoder).
//
// Usage:
//   node extract-grf.mjs --list   <file.grf>
//   node extract-grf.mjs --extract <out-dir> --grf <file.grf> [--match <regex>]
//   node extract-grf.mjs --prune-robes <resources-dir> [--dry-run]
//   node extract-grf.mjs --dump   <file.grf>::<path>      # one file to stdout (fwd-slash path)
//   node extract-grf.mjs --icons  <out-dir> --grf <file.grf> [--iteminfo <path>]
//   node extract-grf.mjs --illust <out-dir> --grf <file.grf>
//   node extract-grf.mjs --effects <out-dir> --grf <file.grf> [--iteminfo <path>]
//   node extract-grf.mjs --maps <out-dir> --grf <file.grf> [--map <name>]
//
// Anywhere a mode takes `--grf <file.grf>` it also takes:
//
//   --grf <file.gpf>    a patch archive (GRF v0x102), carrying `data\**`
//   --grf <file.rgz>    the loose half of a patch: System\**, RagHash.dat, Ragexe
//   --grf <directory>   a mirrored client tree laid out by path (dir/data/...)
//
// The container is detected from its contents, not its extension. Extracting a
// .gpf and a .rgz into the same directory reproduces the client's own layout,
// which is what makes that directory usable as the tree.
//
// Every mode behaves identically against a tree or an archive — verified by
// running --effects both ways over the real client and diffing 3,100 files byte
// for byte.
//
// That matters for incremental updates. A patch archive carries whole copies of
// the files it changes, so applying one is a copy — but only --extract is
// correct when run against a single patch. The derived modes need the whole
// client: --icons resolves item ids through iteminfo and then looks for each
// BMP, --maps needs a map's geometry together with the textures and models it
// references, --illust picks the first card name that resolves to real art.
// Run those against one patch and --effects will rewrite its index.json from
// whatever it happened to resolve, and --illust will quietly pick a different
// name than the merged client would. Pointing them at a mirror avoids all of it.
//
// Examples:
//   # List every entry:
//   node extract-grf.mjs --list data.grf
//
//   # Extract just the resources the gateway needs into ./resources:
//   node extract-grf.mjs --extract resources --grf data.grf \
//     --match "data\\(sprite|palette|imf|luafiles514)\\"
//
//   # The --match value is a JS regex tested (case-insensitive) against each
//   # stored filename. Stored names use BACKSLASH separators, so escape them.
//
//   # Delete the adventurer-backpack sprites Gravity leaves behind in every robe
//   # folder it copies the backpack's to make (the client ignores them; we would
//   # render them instead of the garment). Run this after every --extract:
//   node extract-grf.mjs --prune-robes resources
//
//   # Extract item/collection/skill/job icons (keyed by numeric id) and
//   # char-creation UI elements (keyed by basename) as transparent PNGs
//   # (reads System/iteminfo_new.lub next to the GRF unless --iteminfo is given):
//   node extract-grf.mjs --icons resources/icons --grf data.grf
//
//   # Extract the full-size card artwork (300x400), keyed by item id — the
//   # picture behind a card, which the icons above do NOT contain (every card
//   # shares one generic inventory icon and one generic collection image):
//   node extract-grf.mjs --illust resources/illust --grf data.grf
//
//   # Extract the "effect-only" costumes (auras / falling petals / spotlights —
//   # the costumes that have no character sprite, drawn by the client's ".str"
//   # world-effect system) as per-effect bundles (effect.json + texture PNGs) plus
//   # a catalogue, for the latamvisuais map simulator to fetch like /icons:
//   node extract-grf.mjs --effects resources/effects --grf data.grf
//
//   # Extract every world map (data/<name>.gat/.gnd/.rsw + the .rsm models and
//   # BMP/TGA textures they reference, converted to PNG, plus animated water and
//   # the shared cursor/grid UI) for the latamvisuais 3D map simulator. Models,
//   # textures, water and UI are de-duplicated into content-addressed shared
//   # stores (_t/_m/_w/_u); each map dir holds only its raw .gat/.gnd/.rsw and a
//   # manifest.json referencing the shared blobs. --map limits it to one map:
//   node extract-grf.mjs --maps resources/maps --grf data.grf
//   node extract-grf.mjs --maps resources/maps --grf data.grf --map prontera
//
// Credits: GRF reader, icon pipeline and the mini Lua 5.1 VM extracted from
// adsonpleal/ragreplaystats (tools/build-db.mjs + tools/lua51.mjs).
// The DES routine is ported from vthibault/grf-loader (MIT).

import {
  closeSync,
  copyFileSync,
  existsSync,
  fstatSync,
  mkdirSync,
  openSync,
  readdirSync,
  readFileSync,
  readSync,
  rmSync,
  statSync,
  writeFileSync,
} from "node:fs";
import { resolve, join, dirname } from "node:path";
import { pathToFileURL } from "node:url";
import { deflateSync, gunzipSync, inflateSync } from "node:zlib";
import { createHash } from "node:crypto";

// ---------------------------------------------------------------------------
// CLI
// ---------------------------------------------------------------------------

function parseArgs(argv) {
  const out = {};
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a === "--grf") out.grf = argv[++i];
    else if (a === "--list") out.list = argv[++i];
    else if (a === "--dump") out.dump = argv[++i];
    else if (a === "--extract") out.extract = argv[++i];
    else if (a === "--prune-robes") out.pruneRobes = argv[++i];
    else if (a === "--robe-index") out.robeIndex = argv[++i];
    else if (a === "--index") out.index = argv[++i];
    else if (a === "--dry-run") out.dryRun = true;
    else if (a === "--match") out.match = argv[++i];
    else if (a === "--icons") out.icons = argv[++i];
    else if (a === "--illust") out.illust = argv[++i];
    else if (a === "--effects") out.effects = argv[++i];
    else if (a === "--maps") out.maps = argv[++i];
    else if (a === "--map") out.map = argv[++i];
    else if (a === "--bgm") out.bgm = argv[++i];
    else if (a === "--bgmsrc") out.bgmsrc = argv[++i];
    else if (a === "--sounds") out.sounds = argv[++i];
    else if (a === "--mobids") out.mobids = argv[++i];
    else if (a === "--raw") out.raw = argv[++i];
    else if (a === "--iteminfo") out.iteminfo = argv[++i];
    else if (a === "-h" || a === "--help") out.help = true;
  }
  return out;
}

function usage() {
  console.error(
    [
      "Ragnarok GRF extractor (incl. 0x300 'Event Horizon' + DES decryption)",
      "",
      "  node extract-grf.mjs --list    <file.grf>",
      "  node extract-grf.mjs --extract <out-dir> --grf <file.grf> [--match <regex>]",
      "  node extract-grf.mjs --prune-robes <resources-dir> [--index <f.json>] [--dry-run]",
      "  node extract-grf.mjs --robe-index <out.json> --grf <resources-dir>",
      "  node extract-grf.mjs --dump    <file.grf>::<path>",
      "  node extract-grf.mjs --icons   <out-dir> --grf <file.grf> [--iteminfo <path>]",
      "",
      "  --match is a regex tested against stored names (backslash separators).",
      '  e.g. --match "data\\\\(sprite|palette|imf|luafiles514)\\\\"',
      "",
      "  --prune-robes deletes the adventurer-backpack sprites Gravity leaves in",
      "  every robe folder it copies that folder to make. Run it after --extract.",
      "",
      "  --robe-index records path->hash for the robe tree, and --prune-robes",
      "  --index reuses it. The decision only ever compares hashes, so this lets",
      "  a pipeline work out which files to drop without the 4.5 GB of sprites:",
      "  ~31s hashing the tree once versus ~0.2s per run thereafter.",
      "",
      "  --icons extracts item/collection/skill/job icons (keyed by numeric id)",
      "  and char-creation UI elements (keyed by basename) as transparent PNGs.",
      "  Reads System/iteminfo_new.lub next to the GRF unless --iteminfo points",
      "  at it explicitly.",
      "",
      "  --illust extracts the full-size (300x400) card artwork as <out>/card/<id>.png,",
      "  keyed by item id through data/num2cardillustnametable.txt. Cards all share one",
      "  generic --icons image, so this is the only per-card picture the client ships.",
      "",
      "  --effects extracts the effect-only costumes (.str world effects: auras,",
      "  falling petals, spotlights) as per-effect bundles (effect.json + tex PNGs)",
      "  plus a catalogue (index.json). Also reads iteminfo_new.lub. It additionally",
      "  builds a bundle for every in-world map effect (.rsw type-4 .str, e.g.",
      "  bubble1..4) referenced by EFFECT_STR_TABLE (roBrowser's EffectTable.js).",
      "  It also writes stones.json, the graphic-stone catalogue: each Malangdo",
      "  visual-enchant stone that plays a .str hat effect, mapped to its bundle",
      "  key (STONE_HAT_EFFECT → HatEffectInfo.lub → .str), and footprints.json,",
      "  the stones that instead stamp a decal per footstep — four bundle keys",
      "  (ground mark and puff, per foot) plus the client's placement numbers.",
      "",
      "  --maps extracts every world map (or one, with --map <name>) for the map",
      "  simulator: per-map <name>/{<name>.gat,.gnd,.rsw,manifest.json} plus shared,",
      "  content-addressed model/texture/water/UI stores (_m/_t/_w/_u) and index.json.",
      "  The manifest's `effects` array lists the map's in-world .str effects.",
      "",
      "  --bgm extracts every map's background music: reads data/mp3nametable.txt",
      "  from the GRF and copies the referenced .mp3 files from the client BGM folder",
      "  (next to the GRF, or --bgmsrc <dir>) into <out>/, with index.json mapping",
      "  each map name → its mp3 basename.",
      "",
      "  --sounds extracts the whole data/wav/ tree (skill/effect/monster sound",
      "  effects the /effect/table references by its `wav` field) into <out>/,",
      "  mirroring the GRF paths (effect/ef_portal.wav, _heal_effect.wav, …). PCM",
      "  wavs are copied verbatim; MS/IMA ADPCM wavs are transcoded to 16-bit PCM so",
      "  every sound is browser-playable. Writes index.json listing the names present.",
      "",
      "  --mobids writes the client's monster id universe (id + AEGIS name) from",
      "  datainfo/npcidentity.lub as JSON. It is the candidate list tools/scrape-mobs.mjs",
      "  feeds to the RagnaPlace API, which has no bulk mob endpoint of its own.",
      "",
      "  --raw writes the client data tables the other projects consume — items.json,",
      "  jobs.json, skills.json, randomopt.json, status.json, classes.json (the",
      "  playable classes with their clothes-color palettes, alternative outfits and",
      "  render ids) and hair.json (hair styles + dye swatches per race/gender) —",
      "  into <out-dir> (normally resources/raw, served at /raw/<name>.json).",
      "  They are a faithful projection of the client: per-project naming overrides",
      "  and reshaping stay in each consumer's own sync step. The exception is",
      "  skills.json, which also names the skill ids the client leaves unnamed",
      "  (a name the client ships always wins) and marks follow-up hits with",
      "  `parent`.",
      "  A box's item row carries `contains`: its drop list (id + the client's raw",
      "  prob weight + group). Rows that are not boxes leave the key out entirely.",
    ].join("\n"),
  );
}

function main() {
  const args = parseArgs(process.argv.slice(2));

  if (args.help || (!args.list && !args.extract && !args.pruneRobes && !args.robeIndex && !args.dump && !args.icons && !args.illust && !args.effects && !args.maps && !args.bgm && !args.sounds && !args.mobids && !args.raw)) {
    usage();
    process.exit(args.help ? 0 : 1);
  }

  if (args.list) {
    const grf = openGrf(args.list);
    for (const f of grf.files) {
      console.log(`${f.filename}\t${f.uncompSize}\tflags=0x${f.flags.toString(16)}`);
    }
    closeGrf(grf);
    process.exit(0);
  }

  if (args.extract) {
    if (!args.grf) {
      console.error("usage: --extract <out-dir> --grf <file.grf> [--match <regex>]");
      process.exit(1);
    }
    extractAll(args.grf, args.extract, args.match);
    process.exit(0);
  }

  if (args.pruneRobes) {
    pruneRobes(args.pruneRobes, { dryRun: args.dryRun, indexPath: args.index });
    process.exit(process.exitCode || 0);
  }

  if (args.robeIndex) {
    // --robe-index <out.json> --grf <resources-dir>
    const src = args.grf;
    if (!src) {
      console.error("--robe-index needs --grf <resources-dir>");
      process.exit(1);
    }
    const index = buildRobeIndex(src);
    const total = Object.values(index.folders).reduce((n, f) => n + f.sprites.length, 0);
    writeFileSync(args.robeIndex, JSON.stringify(index));
    console.error(
      `Wrote ${args.robeIndex}: ${Object.keys(index.folders).length} folders, ${total} sprites.`,
    );
    process.exit(0);
  }

  if (args.dump) {
    const [grfPath, wantPath] = args.dump.split("::");
    if (!grfPath || !wantPath) {
      console.error("usage: --dump <file.grf>::<path>");
      process.exit(1);
    }
    const grf = openGrf(grfPath);
    try {
      const want = normalize(wantPath);
      const entry = findBestEntry(grf, want);
      if (!entry) {
        console.error(`Not found: ${wantPath}`);
        process.exit(1);
      }
      process.stdout.write(Buffer.from(extractFile(grf, entry)));
    } finally {
      closeGrf(grf);
    }
    process.exit(0);
  }

  if (args.icons) {
    if (!args.grf) {
      console.error("usage: --icons <out-dir> --grf <file.grf> [--iteminfo <path>]");
      process.exit(1);
    }
    extractIcons(args.grf, args.icons, args);
    process.exit(0);
  }

  if (args.illust) {
    if (!args.grf) {
      console.error("usage: --illust <out-dir> --grf <file.grf>");
      process.exit(1);
    }
    extractIllust(args.grf, args.illust);
    process.exit(0);
  }

  if (args.effects) {
    if (!args.grf) {
      console.error("usage: --effects <out-dir> --grf <file.grf> [--iteminfo <path>]");
      process.exit(1);
    }
    extractEffects(args.grf, args.effects, args);
    process.exit(0);
  }

  if (args.maps) {
    if (!args.grf) {
      console.error("usage: --maps <out-dir> --grf <file.grf> [--map <name>]");
      process.exit(1);
    }
    extractMaps(args.grf, args.maps, args);
    process.exit(0);
  }

  if (args.bgm) {
    if (!args.grf) {
      console.error("usage: --bgm <out-dir> --grf <file.grf> [--bgmsrc <BGM-dir>]");
      process.exit(1);
    }
    extractBgm(args.grf, args.bgm, args);
    process.exit(0);
  }

  if (args.sounds) {
    if (!args.grf) {
      console.error("usage: --sounds <out-dir> --grf <file.grf>");
      process.exit(1);
    }
    extractSounds(args.grf, args.sounds, args);
    process.exit(0);
  }

  if (args.mobids) {
    if (!args.grf) {
      console.error("usage: --mobids <out.json> --grf <file.grf>");
      process.exit(1);
    }
    extractMobIds(args.grf, args.mobids);
    process.exit(0);
  }

  if (args.raw) {
    if (!args.grf) {
      console.error("usage: --raw <out-dir> --grf <file.grf> [--iteminfo <path>]");
      process.exit(1);
    }
    extractRawTables(args.grf, args.raw, args);
    process.exit(0);
  }
}

// ---------------------------------------------------------------------------
// Path helpers
// ---------------------------------------------------------------------------

function normalize(s) {
  return s.replace(/[\\/]+/g, "/").toLowerCase();
}

// The merged "Event Horizon" GRF carries several copies of the same logical
// path (patch layering, double-slash artifacts, etc.). normalize() collapses
// repeated slashes so they compare equal; among matches keep the largest by
// uncompressed size, which is the complete, non-truncated copy in practice.
function findBestEntry(grf, want) {
  let best = null;
  for (const f of grf.files) {
    if (!(f.flags & 0x01)) continue;
    if (!normalize(f.filename).endsWith(want)) continue;
    if (!best || f.uncompSize > best.uncompSize) best = f;
  }
  return best;
}

// sanitizePath turns an archive entry name into the relative path --extract
// writes it to.
//
// Everything under data/ is folded to lowercase. data.grf itself ships every
// name lowercase, so that is the mirror's convention and what the renderer and
// every mode here look paths up by — but patches ship Gravity's own spelling
// (data/sprite/악세사리/남/남_C_CLB_BS_Hood_B.Spr). On a case-sensitive disk an
// unfolded patch file is invisible to the renderer when new, and when it
// replaces an existing file it lands beside the old copy, which keeps being
// served. Folding here, per archive, also keeps sequence order meaningful when
// two patches spell the same file differently.
//
// The loose files outside data/ (System/itemInfo.lua, BGM/) keep their case:
// they mirror a client install, and are read by their real names.
export function sanitizePath(name) {
  let s = name.replace(/\\/g, "/").replace(/^\/+/, "");
  if (/^[A-Za-z]:/.test(s)) s = s.slice(2).replace(/^\/+/, "");
  if (!s) return null;
  for (const part of s.split("/")) {
    if (part === ".." || part === ".") return null;
  }
  if (/^data\//i.test(s)) s = s.toLowerCase();
  return s;
}

function decodeName(bytes) {
  try {
    return new TextDecoder("euc-kr", { fatal: true }).decode(bytes);
  } catch {
    return new TextDecoder("windows-1252").decode(bytes);
  }
}

// ---------------------------------------------------------------------------
// GRF reader (versions 0x101, 0x103, 0x200, and custom 0x300 forks)
// ---------------------------------------------------------------------------

function openGrf(path) {
  // A directory is a mirrored client tree rather than an archive. Dispatching
  // here rather than adding a flag means every mode accepts one through the
  // existing --grf, with no change to the eleven places that open an archive:
  // they get a handle either way and cannot tell the difference.
  if (existsSync(path) && statSync(path).isDirectory()) {
    console.error(`Tree: ${resolve(path)}`);
    const tree = openTree({ root: path });
    console.error(`${tree.files.length} files in mirrored client tree`);
    return tree;
  }

  const fd = openSync(path, "r");
  const fileSize = fstatSync(fd).size;

  // A .rgz is a gzip stream, not a GRF. Sniff the magic rather than trusting the
  // extension: patch filenames follow their own convention and a mislabelled one
  // should still open. Dispatched here for the same reason directories are —
  // every mode gets a handle and none of them has to care.
  const magic2 = Buffer.alloc(2);
  if (fileSize >= 2) readAt(fd, magic2, 0);
  if (magic2[0] === 0x1f && magic2[1] === 0x8b) {
    closeSync(fd);
    return openRgz(path);
  }

  const header = Buffer.alloc(0x2e);
  readAt(fd, header, 0);
  const magic = header.toString("ascii", 0, 16).replace(/\0.*$/, "");
  console.error(`Magic: "${magic}"`);
  const filetableOffset = header.readUInt32LE(0x1e);
  const m1 = header.readUInt32LE(0x22);
  const m2 = header.readUInt32LE(0x26);
  const version = header.readUInt32LE(0x2a);
  const fileCount = m2 - m1 - 7;
  console.error(
    `GRF version 0x${version.toString(16)}, ${fileCount} files (~${(fileSize / 1024 / 1024 / 1024).toFixed(2)} GB), table at 0x${filetableOffset.toString(16)}`,
  );

  let files;
  if (version === 0x200) {
    files = readFileTableV200(fd, 0x2e + filetableOffset);
  } else if (version === 0x300) {
    // Custom forks (Event Horizon etc.) — 4-byte gap before the compressed
    // table and a 21-byte entry trailer (extra u32 vs v0x200).
    files = readFileTableV200(fd, 0x32 + filetableOffset, 21);
  } else if (version === 0x103) {
    files = readFileTableV103(fd, 0x2e + filetableOffset, fileCount, fileSize);
  } else if (version === 0x102 || version === 0x101 || version === 0x100) {
    // 0x101 used to be routed to the 0x103 reader. That could not have worked:
    // v1 encrypts filenames and offsets the size fields, so the 0x103 reader
    // yields garbage names and sizes that fail to inflate. Only 0x102 is
    // verified against real archives (every LATAM .gpf patch); 0x100/0x101 come
    // from the format spec and share the layout, but no archive was available to
    // test them against.
    files = readFileTableV1(fd, 0x2e + filetableOffset, fileCount, fileSize);
  } else {
    closeSync(fd);
    throw new Error(`Unsupported GRF version 0x${version.toString(16)}`);
  }
  return {
    fd,
    fileSize,
    version,
    files,
    read: (entry) => readGrfEntry(fd, entry),
    close: () => closeSync(fd),
  };
}

function readAt(fd, buf, position) {
  let read = 0;
  while (read < buf.length) {
    const n = readSync(fd, buf, read, buf.length - read, position + read);
    if (n <= 0) break;
    read += n;
  }
  return read;
}

function readBytes(fd, length, position) {
  const buf = Buffer.alloc(length);
  readAt(fd, buf, position);
  return buf;
}

function readFileTableV200(fd, tableStart, entryTrailerBytes = 17) {
  const sizes = readBytes(fd, 8, tableStart);
  const compressedSize = sizes.readUInt32LE(0);
  const uncompressedSize = sizes.readUInt32LE(4);
  const compressed = readBytes(fd, compressedSize, tableStart + 8);
  const table = inflateSync(compressed);
  if (table.length !== uncompressedSize) {
    console.warn(`! filetable inflate size ${table.length} != expected ${uncompressedSize}`);
  }
  const files = [];
  let p = 0;
  while (p < table.length) {
    const nullIdx = table.indexOf(0, p);
    if (nullIdx < 0) break;
    const filename = decodeName(table.subarray(p, nullIdx));
    p = nullIdx + 1;
    if (p + entryTrailerBytes > table.length) break;
    const compSize = table.readUInt32LE(p);
    const compSizeAligned = table.readUInt32LE(p + 4);
    const uncompSize = table.readUInt32LE(p + 8);
    const flags = table.readUInt8(p + 12);
    // The 0x300 "Event Horizon" fork stores a 64-bit data offset (its 21-byte
    // trailer = the standard 17 + a high u32), so files appended past the 4 GB
    // mark — recent patches — resolve correctly. v0x200 is 32-bit.
    const offsetLow = table.readUInt32LE(p + 13);
    const offsetHigh = entryTrailerBytes >= 21 ? table.readUInt32LE(p + 17) : 0;
    const offset = offsetHigh * 0x100000000 + offsetLow;
    p += entryTrailerBytes;
    files.push({ filename, compSize, compSizeAligned, uncompSize, flags, offset });
  }
  return files;
}

function readFileTableV103(fd, tableStart, fileCount, fileSize) {
  const buf = readBytes(fd, fileSize - tableStart, tableStart);
  const files = [];
  let p = 0;
  for (let i = 0; i < fileCount && p < buf.length; i++) {
    const len = buf.readUInt32LE(p);
    p += 4;
    const filename = decodeName(buf.subarray(p + 2, p + 2 + len - 6));
    p += len;
    if (p + 17 > buf.length) break;
    const compSize = buf.readUInt32LE(p);
    const compSizeAligned = buf.readUInt32LE(p + 4);
    const uncompSize = buf.readUInt32LE(p + 8);
    const flags = buf.readUInt8(p + 12);
    const offset = buf.readUInt32LE(p + 13);
    p += 17;
    files.push({ filename, compSize, compSizeAligned, uncompSize, flags, offset });
  }
  return files;
}

// ---------------------------------------------------------------------------
// GRF v1 (0x100 / 0x101 / 0x102) file table.
//
// This is the format of every LATAM `.gpf` patch archive, and of the client's
// event.grf. The extractor only ever opened the merged client data.grf (0x300)
// before, which is why it never needed this. The entry layout matches 0x103 —
// [u32 len][2 bytes][name: len-6][u32 x3][u8 flags][u32 offset] — but three
// things differ, and all three have to be handled together or the archive
// decodes to garbage rather than failing cleanly:
//
//  1. FILENAMES ARE ENCRYPTED. Each 8-byte block is nibble-swapped
//     (b = b<<4 | b>>4) and *then* run through the single-round DES block. The
//     order matters: DES-then-swap yields nonsense. It is keyless ECB, which is
//     why sibling entries visibly share leading blocks.
//     Verified: cipher 03 56 47 52 80 90 02 43 -> "data\Lua".
//
//  2. THE SIZE FIELDS ARE OFFSET, not stored raw:
//         uncompSize      = raw3
//         compSize        = raw1 - raw3 - 715
//         compSizeAligned = raw2 - 37579
//     Verified on a real 4-entry .gpf: aligned resolves to 26032, and
//     0x2e + 0 + 26032 lands exactly on the file table offset.
//
//  3. THE FLAGS BYTE LIES. Every entry reports plain 0x01; the DES pass has to
//     be derived from the extension instead. Header-only (0x04) for the formats
//     the client streams (.gnd/.gat/.act/.str), mixed (0x02) for everything
//     else. Established by brute-forcing both passes over real archives.
// ---------------------------------------------------------------------------

// Decrypts a v1 entry name in place: nibble-swap each byte of an 8-byte block,
// then DES it. Exported for tests.
export function decodeFilenameV1(buf) {
  for (let i = 0; i + 8 <= buf.length; i += 8) {
    for (let j = i; j < i + 8; j++) buf[j] = ((buf[j] << 4) | (buf[j] >> 4)) & 0xff;
    desDecryptBlock(buf, i);
  }
  return buf;
}

// v1EncryptionFlags derives the DES pass from the extension, because the flags
// byte in a v1 entry does not carry it — every file reports a plain 0x01. The
// split is between the formats the client streams and everything else, and was
// established by brute-forcing both passes over real archives rather than read
// off a spec. Exported for tests.
export function v1EncryptionFlags(filename, flags) {
  const FILE_BIT = 0x01;
  if (!(flags & FILE_BIT)) return flags; // directory entry: no payload to decrypt
  const ext = filename.slice(-4).toLowerCase();
  const headerOnly = ext === ".gnd" || ext === ".gat" || ext === ".act" || ext === ".str";
  return flags | (headerOnly ? 0x04 /* ENC_HEADER */ : 0x02 /* ENC_MIXED */);
}

function readFileTableV1(fd, tableStart, fileCount, fileSize) {
  return parseFileTableV1(readBytes(fd, fileSize - tableStart, tableStart), fileCount);
}

// parseFileTableV1 is the pure half of readFileTableV1: everything but the read.
// Split out so the entry layout, the name decryption, the size de-offsetting and
// the extension-derived DES mode can be tested against a hand-built table,
// without shipping a real .gpf — this project distributes no game assets.
export function parseFileTableV1(buf, fileCount) {
  const files = [];
  let p = 0;
  for (let i = 0; i < fileCount && p + 4 <= buf.length; i++) {
    const len = buf.readUInt32LE(p);
    p += 4;
    if (len < 6 || p + len > buf.length) break;
    // Copy: decodeFilenameV1 decrypts in place and the source buffer is shared.
    const nameBuf = Buffer.from(buf.subarray(p + 2, p + 2 + len - 6));
    decodeFilenameV1(nameBuf);
    // The encrypted name is padded to a multiple of 8, so it can carry trailing
    // NULs that must not become part of the path.
    const nul = nameBuf.indexOf(0);
    const filename = decodeName(nul < 0 ? nameBuf : nameBuf.subarray(0, nul));
    p += len;
    if (p + 17 > buf.length) break;
    const raw1 = buf.readUInt32LE(p);
    const raw2 = buf.readUInt32LE(p + 4);
    const uncompSize = buf.readUInt32LE(p + 8);
    let flags = buf.readUInt8(p + 12);
    const offset = buf.readUInt32LE(p + 13);
    p += 17;
    flags = v1EncryptionFlags(filename, flags);
    files.push({
      filename,
      compSize: raw1 - uncompSize - 715,
      compSizeAligned: raw2 - 37579,
      uncompSize,
      flags,
      offset,
    });
  }
  return files;
}

// ---------------------------------------------------------------------------
// GRF DES decryption — Ragnarok's custom single-round DES with block cycling
// and a byte shuffle. Ported from grf-loader (vthibault/grf-loader, MIT).
// Encrypted entries are flagged ENC_MIXED (0x02 — header DES + periodic
// DES/shuffle) or ENC_HEADER (0x04 — first 20 blocks DES only). Both operate
// on the *compressed* bytes in place, before inflate.
// ---------------------------------------------------------------------------

const DES_MASK = new Uint8Array([0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01]);
const _t = new Uint8Array(8);
const _t2 = new Uint8Array(8);
const _zero = new Uint8Array(8);

// prettier-ignore
const DES_IP = new Uint8Array([
  58,50,42,34,26,18,10,2, 60,52,44,36,28,20,12,4,
  62,54,46,38,30,22,14,6, 64,56,48,40,32,24,16,8,
  57,49,41,33,25,17,9,1,  59,51,43,35,27,19,11,3,
  61,53,45,37,29,21,13,5, 63,55,47,39,31,23,15,7,
]);
// prettier-ignore
const DES_FP = new Uint8Array([
  40,8,48,16,56,24,64,32, 39,7,47,15,55,23,63,31,
  38,6,46,14,54,22,62,30, 37,5,45,13,53,21,61,29,
  36,4,44,12,52,20,60,28, 35,3,43,11,51,19,59,27,
  34,2,42,10,50,18,58,26, 33,1,41,9,49,17,57,25,
]);
// prettier-ignore
const DES_TP = new Uint8Array([
  16,7,20,21, 29,12,28,17, 1,15,23,26, 5,18,31,10,
  2,8,24,14,  32,27,3,9,   19,13,30,6,  22,11,4,25,
]);
// prettier-ignore
const DES_SBOX = [
  new Uint8Array([
    0xef,0x03,0x41,0xfd,0xd8,0x74,0x1e,0x47, 0x26,0xef,0xfb,0x22,0xb3,0xd8,0x84,0x1e,
    0x39,0xac,0xa7,0x60,0x62,0xc1,0xcd,0xba, 0x5c,0x96,0x90,0x59,0x05,0x3b,0x7a,0x85,
    0x40,0xfd,0x1e,0xc8,0xe7,0x8a,0x8b,0x21, 0xda,0x43,0x64,0x9f,0x2d,0x14,0xb1,0x72,
    0xf5,0x5b,0xc8,0xb6,0x9c,0x37,0x76,0xec, 0x39,0xa0,0xa3,0x05,0x52,0x6e,0x0f,0xd9,
  ]),
  new Uint8Array([
    0xa7,0xdd,0x0d,0x78,0x9e,0x0b,0xe3,0x95, 0x60,0x36,0x36,0x4f,0xf9,0x60,0x5a,0xa3,
    0x11,0x24,0xd2,0x87,0xc8,0x52,0x75,0xec, 0xbb,0xc1,0x4c,0xba,0x24,0xfe,0x8f,0x19,
    0xda,0x13,0x66,0xaf,0x49,0xd0,0x90,0x06, 0x8c,0x6a,0xfb,0x91,0x37,0x8d,0x0d,0x78,
    0xbf,0x49,0x11,0xf4,0x23,0xe5,0xce,0x3b, 0x55,0xbc,0xa2,0x57,0xe8,0x22,0x74,0xce,
  ]),
  new Uint8Array([
    0x2c,0xea,0xc1,0xbf,0x4a,0x24,0x1f,0xc2, 0x79,0x47,0xa2,0x7c,0xb6,0xd9,0x68,0x15,
    0x80,0x56,0x5d,0x01,0x33,0xfd,0xf4,0xae, 0xde,0x30,0x07,0x9b,0xe5,0x83,0x9b,0x68,
    0x49,0xb4,0x2e,0x83,0x1f,0xc2,0xb5,0x7c, 0xa2,0x19,0xd8,0xe5,0x7c,0x2f,0x83,0xda,
    0xf7,0x6b,0x90,0xfe,0xc4,0x01,0x5a,0x97, 0x61,0xa6,0x3d,0x40,0x0b,0x58,0xe6,0x3d,
  ]),
  new Uint8Array([
    0x4d,0xd1,0xb2,0x0f,0x28,0xbd,0xe4,0x78, 0xf6,0x4a,0x0f,0x93,0x8b,0x17,0xd1,0xa4,
    0x3a,0xec,0xc9,0x35,0x93,0x56,0x7e,0xcb, 0x55,0x20,0xa0,0xfe,0x6c,0x89,0x17,0x62,
    0x17,0x62,0x4b,0xb1,0xb4,0xde,0xd1,0x87, 0xc9,0x14,0x3c,0x4a,0x7e,0xa8,0xe2,0x7d,
    0xa0,0x9f,0xf6,0x5c,0x6a,0x09,0x8d,0xf0, 0x0f,0xe3,0x53,0x25,0x95,0x36,0x28,0xcb,
  ]),
];

const DES_SHUFFLE = (() => {
  const list = new Uint8Array([
    0x00, 0x2b, 0x6c, 0x80, 0x01, 0x68, 0x48,
    0x77, 0x60, 0xff, 0xb9, 0xc0, 0xfe, 0xeb,
  ]);
  const out = new Uint8Array(256);
  for (let i = 0; i < 256; i++) out[i] = i;
  for (let i = 0; i < list.length; i += 2) {
    out[list[i]] = list[i + 1];
    out[list[i + 1]] = list[i];
  }
  return out;
})();

function desInitialPerm(src, index) {
  for (let i = 0; i < 64; ++i) {
    const j = DES_IP[i] - 1;
    if (src[index + ((j >> 3) & 7)] & DES_MASK[j & 7]) _t[(i >> 3) & 7] |= DES_MASK[i & 7];
  }
  src.set(_t, index);
  _t.set(_zero);
}

function desFinalPerm(src, index) {
  for (let i = 0; i < 64; ++i) {
    const j = DES_FP[i] - 1;
    if (src[index + ((j >> 3) & 7)] & DES_MASK[j & 7]) _t[(i >> 3) & 7] |= DES_MASK[i & 7];
  }
  src.set(_t, index);
  _t.set(_zero);
}

function desTransposition(src, index) {
  for (let i = 0; i < 32; ++i) {
    const j = DES_TP[i] - 1;
    if (src[index + (j >> 3)] & DES_MASK[j & 7]) _t[(i >> 3) + 4] |= DES_MASK[i & 7];
  }
  src.set(_t, index);
  _t.set(_zero);
}

function desExpansion(src, index) {
  _t[0] = ((src[index + 7] << 5) | (src[index + 4] >> 3)) & 0x3f;
  _t[1] = ((src[index + 4] << 1) | (src[index + 5] >> 7)) & 0x3f;
  _t[2] = ((src[index + 4] << 5) | (src[index + 5] >> 3)) & 0x3f;
  _t[3] = ((src[index + 5] << 1) | (src[index + 6] >> 7)) & 0x3f;
  _t[4] = ((src[index + 5] << 5) | (src[index + 6] >> 3)) & 0x3f;
  _t[5] = ((src[index + 6] << 1) | (src[index + 7] >> 7)) & 0x3f;
  _t[6] = ((src[index + 6] << 5) | (src[index + 7] >> 3)) & 0x3f;
  _t[7] = ((src[index + 7] << 1) | (src[index + 4] >> 7)) & 0x3f;
  src.set(_t, index);
  _t.set(_zero);
}

function desSbox(src, index) {
  for (let i = 0; i < 4; ++i) {
    _t[i] =
      (DES_SBOX[i][src[i * 2 + 0 + index]] & 0xf0) |
      (DES_SBOX[i][src[i * 2 + 1 + index]] & 0x0f);
  }
  src.set(_t, index);
  _t.set(_zero);
}

function desRound(src, index) {
  for (let i = 0; i < 8; i++) _t2[i] = src[index + i];
  desExpansion(_t2, 0);
  desSbox(_t2, 0);
  desTransposition(_t2, 0);
  src[index + 0] ^= _t2[4];
  src[index + 1] ^= _t2[5];
  src[index + 2] ^= _t2[6];
  src[index + 3] ^= _t2[7];
}

function desDecryptBlock(src, index) {
  desInitialPerm(src, index);
  desRound(src, index);
  desFinalPerm(src, index);
}

function desShuffleDec(src, index) {
  _t[0] = src[index + 3];
  _t[1] = src[index + 4];
  _t[2] = src[index + 6];
  _t[3] = src[index + 0];
  _t[4] = src[index + 1];
  _t[5] = src[index + 2];
  _t[6] = src[index + 5];
  _t[7] = DES_SHUFFLE[src[index + 7]];
  src.set(_t, index);
  _t.set(_zero);
}

// ENC_MIXED: first 20 blocks DES-decrypted; thereafter every `cycle`-th block
// is DES-decrypted and every 7th remaining block is de-shuffled. `entryLength`
// is the *compressed* size and drives the cycle gap.
function desDecodeFull(src, length, entryLength) {
  const digits = entryLength.toString().length;
  const cycle =
    digits < 3 ? 1 : digits < 5 ? digits + 1 : digits < 7 ? digits + 9 : digits + 15;
  const nblocks = length >> 3;
  for (let i = 0; i < 20 && i < nblocks; ++i) desDecryptBlock(src, i * 8);
  for (let i = 20, j = -1; i < nblocks; ++i) {
    if (i % cycle === 0) {
      desDecryptBlock(src, i * 8);
      continue;
    }
    if (++j && j % 7 === 0) desShuffleDec(src, i * 8);
  }
}

// ENC_HEADER: only the first 20 blocks are DES-decrypted; the rest is plaintext.
function desDecodeHeader(src, length) {
  const count = length >> 3;
  for (let i = 0; i < 20 && i < count; ++i) desDecryptBlock(src, i * 8);
}

// ---------------------------------------------------------------------------
// .rgz — Gravity's loose-file patch container, the other half of a patch set.
//
// A patch release ships .gpf archives carrying `data\**` (the data.grf tree) and
// .rgz archives carrying everything that lives loose in the client install:
// `System\**` (iteminfo_new.lub among them), RagHash.dat, Ragexe.exe. That makes
// it load-bearing here rather than optional — --icons and --raw both key off
// iteminfo_new.lub, and it only ever arrives this way.
//
// A .rgz is a gzip stream; inflated it is a flat sequence of records:
//
//   [type:u8][nameLen:u8][name: nameLen bytes, NUL-terminated]
//   ...and for 'f' only: [size:u32 LE][data: size bytes]
//
//   'd' (0x64) directory — name only
//   'f' (0x66) file      — size and payload follow
//   'e' (0x65) end       — sentinel, name is literally "end\0"
//
// Names are backslash-separated and relative to the CLIENT INSTALL ROOT, not to
// data.grf — "System\iteminfo_new.lub", not "data\...". Extracting a .gpf and a
// .rgz into the same directory therefore reproduces the client's own layout,
// which is exactly what openTree wants to be pointed at.
// ---------------------------------------------------------------------------

const RGZ_DIR = 0x64; // 'd'
const RGZ_FILE = 0x66; // 'f'
const RGZ_END = 0x65; // 'e'

/**
 * Parse a .rgz buffer into its records.
 *
 * Returns { entries, inflated, truncated }. `offset` on a file entry is the
 * payload's start within `inflated`, so callers slice lazily instead of copying
 * every file up front. `truncated` reports a stream that ended mid-record rather
 * than throwing: a partially downloaded patch is worth reporting precisely.
 */
export function readRgz(buf) {
  const inflated = gunzipSync(buf);
  const entries = [];
  let p = 0;
  let truncated = false;

  while (p < inflated.length) {
    const type = inflated[p++];

    if (type === RGZ_END) {
      // The sentinel still carries a name field ("end\0"); consume and stop.
      if (p < inflated.length) p += 1 + inflated[p];
      break;
    }

    if (type !== RGZ_DIR && type !== RGZ_FILE) {
      throw new Error(`rgz: unknown record type 0x${type.toString(16)} at offset ${p - 1}`);
    }

    if (p >= inflated.length) {
      truncated = true;
      break;
    }
    const nameLen = inflated[p++];
    if (p + nameLen > inflated.length) {
      truncated = true;
      break;
    }
    // nameLen counts the trailing NUL, so drop it.
    const name = decodeName(inflated.subarray(p, p + nameLen - 1));
    p += nameLen;

    if (type === RGZ_DIR) {
      entries.push({ type: "dir", name, size: 0, offset: -1 });
      continue;
    }

    if (p + 4 > inflated.length) {
      truncated = true;
      break;
    }
    const size = inflated.readUInt32LE(p);
    p += 4;
    entries.push({ type: "file", name, size, offset: p });
    if (p + size > inflated.length) {
      truncated = true;
      break;
    }
    p += size;
  }

  return { entries, inflated, truncated };
}

/** Slice one entry's payload out of the inflated stream. */
export function rgzData(parsed, entry) {
  if (entry.type !== "file") return Buffer.alloc(0);
  return parsed.inflated.subarray(entry.offset, entry.offset + entry.size);
}

// openRgz presents a .rgz with the same shape openGrf returns, so --list,
// --extract and every other mode work on one unchanged.
function openRgz(path) {
  const parsed = readRgz(readFileSync(path));
  if (parsed.truncated) {
    console.error("Warning: .rgz stream ends mid-record — the archive is truncated.");
  }
  const files = parsed.entries
    .filter((e) => e.type === "file")
    .map((e) => ({
      filename: e.name,
      rgzEntry: e,
      flags: 0x01, // plaintext: .rgz carries no per-entry encryption
      uncompSize: e.size,
      compSize: e.size,
      compSizeAligned: e.size,
      offset: e.offset,
    }));
  console.error(`RGZ: ${files.length} file(s), ${parsed.entries.length} record(s)`);
  return {
    files,
    version: 0,
    read: (entry) =>
      entry.flags & 0x01 ? rgzData(parsed, entry.rgzEntry) : new Uint8Array(0),
    close() {},
  };
}

// ---------------------------------------------------------------------------
// openTree — a merged client view that stands in for an opened GRF.
//
// Every mode except --extract needs to see the *whole* client, not one archive.
// --icons maps item ids to resource names through System/iteminfo_new.lub and
// then looks for each BMP; --maps needs a map's .gat/.gnd/.rsw together with the
// textures and models they reference; --illust picks the first card name that
// resolves to real art. Run any of those against a single patch and they do not
// merely come up short — --effects rewrites index.json from whatever it happened
// to resolve, clobbering the full catalogue, and --illust silently picks a
// different name than the merged view would.
//
// So the pipeline keeps a mirror of the client as a directory tree (patches carry
// whole files, never binary deltas, so applying one is a copy) and hands it here.
// openTree presents that directory with the same shape openGrf returns, which is
// why the modes need no changes at all: they walk `files` and call extractFile,
// and neither cares whether the bytes come from an archive or a file.
//
// Deliberately synchronous and local. The extractor is sync throughout
// (readFileSync/writeFileSync), so a read that went to the network would mean
// making every mode async. Hydrating the tree is the caller's job; a read of a
// path that is not present throws, naming it, so a pipeline can see exactly what
// it still needs to fetch.
// ---------------------------------------------------------------------------

// treeEntryName converts a filesystem-relative path to the backslash-separated,
// GRF-style name every mode matches against (--match patterns, buildEntryIndex,
// findBestEntry and sanitizePath all expect that form).
function treeEntryName(relPath) {
  return relPath.split("/").join("\\");
}

// walkTree lists every file under root, relative and forward-slashed.
function walkTree(root, rel = "", out = []) {
  let entries;
  try {
    entries = readdirSync(join(root, rel), { withFileTypes: true });
  } catch {
    return out;
  }
  for (const e of entries) {
    const child = rel ? `${rel}/${e.name}` : e.name;
    if (e.isDirectory()) walkTree(root, child, out);
    else if (e.isFile()) out.push(child);
  }
  return out;
}

/**
 * openTree({ root, index }) presents a directory as an opened archive.
 *
 * root  — directory holding the mirrored client, laid out by path (root/data/...).
 * index — optional array of forward-slashed relative paths to expose instead of
 *         walking the tree. Use it when the mirror is only partially hydrated but
 *         the full file list is known, so modes still enumerate the real client
 *         rather than whatever happens to be on disk.
 *
 * Entries carry the file bit so the `flags & 0x01` filter every mode applies
 * keeps them, and no encryption bits, because the bytes are already plaintext.
 */
export function openTree({ root, index = null }) {
  const base = resolve(root);
  const rels = index ?? walkTree(base);
  const files = rels.map((rel) => ({
    filename: treeEntryName(rel),
    treePath: rel,
    flags: 0x01,
    // Sizes are reported for parity with a GRF entry. They are only consulted
    // for progress reporting and the stored-vs-deflated check, which does not
    // apply here, so a stat is not worth doing up front for a 260k-file mirror.
    uncompSize: 0,
    compSize: 0,
    compSizeAligned: 0,
    offset: 0,
  }));

  return {
    files,
    version: 0,
    read(entry) {
      if (!(entry.flags & 0x01)) return new Uint8Array(0);
      const rel = entry.treePath ?? entry.filename.split("\\").join("/");
      const p = join(base, rel);
      try {
        return readFileSync(p);
      } catch (err) {
        // Naming the missing path is the point: a partially hydrated mirror is
        // an expected state, and the caller needs to know what to fetch.
        throw new Error(`not hydrated in tree: ${rel} (${err.code ?? err.message})`);
      }
    },
    close() {},
  };
}

// ---------------------------------------------------------------------------
// Extraction
// ---------------------------------------------------------------------------

// readGrfEntry pulls one entry's bytes out of an open archive: read the raw
// span, undo whichever DES pass the entry took, then inflate unless it was
// stored. This is openGrf's read(); nothing else should touch grf.fd.
function readGrfEntry(fd, entry) {
  const FILE_BIT = 0x01;
  const ENC_MIXED = 0x02;
  const ENC_HEADER = 0x04;
  if (!(entry.flags & FILE_BIT)) return new Uint8Array(0);
  const raw = readBytes(fd, entry.compSizeAligned, 0x2e + entry.offset);
  if (entry.flags & ENC_MIXED) desDecodeFull(raw, entry.compSizeAligned, entry.compSize);
  else if (entry.flags & ENC_HEADER) desDecodeHeader(raw, entry.compSizeAligned);
  // Stored (not deflated) when compressed size == real size.
  if (entry.uncompSize === entry.compSize) return raw;
  return inflateSync(raw);
}

// extractFile and closeGrf are the only things every mode uses to get at an
// archive's bytes, so they dispatch through the handle rather than reaching for
// a file descriptor. That is what lets openTree stand in for openGrf: the ~15
// places that walk `grf.files` and call extractFile do not care which one they
// were given.
function extractFile(grf, entry) {
  return grf.read(entry);
}

function closeGrf(grf) {
  grf?.close?.();
}

function extractAll(grfPath, outDir, matchPattern) {
  const grf = openGrf(grfPath);
  const re = matchPattern ? new RegExp(matchPattern, "i") : null;
  const root = resolve(outDir);
  if (!existsSync(root)) mkdirSync(root, { recursive: true });

  const startedAt = Date.now();
  let written = 0;
  let skipped = 0;
  let encrypted = 0;
  let bytes = 0;

  try {
    let lastReportAt = startedAt;
    for (let i = 0; i < grf.files.length; i++) {
      const entry = grf.files[i];
      if (!(entry.flags & 0x01)) continue;
      if (re && !re.test(entry.filename)) continue;
      if (entry.flags & 0x06) encrypted++; // decrypted in extractFile; just track count

      const safe = sanitizePath(entry.filename);
      if (!safe) {
        skipped++;
        continue;
      }
      const dest = join(root, safe);
      const dir = dirname(dest);
      try {
        if (dir && !existsSync(dir)) mkdirSync(dir, { recursive: true });
        const data = extractFile(grf, entry);
        writeFileSync(dest, data);
        written++;
        bytes += data.length;
      } catch {
        skipped++;
      }

      const now = Date.now();
      if (now - lastReportAt > 2000) {
        const pct = ((i / grf.files.length) * 100).toFixed(1);
        console.error(
          `  [${pct}%] ${written} written, ${skipped} skipped, ${encrypted} encrypted, ${(bytes / 1e6).toFixed(0)} MB`,
        );
        lastReportAt = now;
      }
    }
  } finally {
    closeGrf(grf);
  }

  const dur = ((Date.now() - startedAt) / 1000).toFixed(1);
  console.error(
    `\nExtracted ${written} file(s), ${(bytes / 1e9).toFixed(2)} GB to ${root} in ${dur}s.`,
  );
  if (encrypted) console.error(`Decrypted ${encrypted} encrypted file(s).`);
  if (skipped) console.error(`Skipped ${skipped} unreadable/invalid file(s).`);
}

// ---------------------------------------------------------------------------
// Robe template leftovers (--prune-robes)
// ---------------------------------------------------------------------------
//
// Gravity builds each 로브/<garment>/ folder by copying the 모험가배낭 ("Adventurer's
// Backpack") folder: every per-job .act is replaced with the new garment's
// geometry and so is the folder-root .spr, but the per-job .spr files are left
// behind as the backpack's. The client never reads them — it pairs a per-job .act
// with the folder-root .spr — so nobody at Gravity notices.
//
// Our renderer does read them: engine.loadGarment takes the first candidate pair
// where both files exist, and resolve.GarmentCandidates offers {per-job act,
// per-job spr} before {per-job act, root spr}. So every job that inherited a
// leftover renders the backpack instead of the garment. Only jobs the folder has
// no per-job .spr for (the 4th classes, which are act-only here) came out right.
//
// The fix is to delete the leftovers from the extracted tree; the {per-job act,
// root spr} pair already in the candidate list then takes over on its own, and no
// resolver change is needed. This runs on the extracted tree rather than inside
// --extract so that --extract stays a plain byte copier, and so an already
// extracted resources/ (including a deployed one) can be repaired in place.
//
// "The root .spr always wins" would be the easy rule and it is WRONG: 201 of the
// client's 218 robe folders are healthy, and plenty of them (c_giant_white_rabbit,
// c_niflheim_key, c_samba_carnival) ship genuine per-job image banks that differ
// from their root .spr and must keep winning. The criterion has to be the
// content: is this .spr the backpack?

const kROBE = "로브";
const kGENDER_DIRS = ["남", "여"];

// A per-job .spr content this many distinct robe folders share is not any one
// garment's artwork — it is a copy left over from the template. In this client
// the measurement is unambiguous: 8 contents appear in 18–20 folders each and the
// next-largest sharing group is 4, so anything from 5 to 17 selects the same 8.
// All 8 decode to the adventurer's backpack (7 of them are the same bag drawn for
// a different body — the per-job variants 모험가배낭's own folder no longer ships,
// which is why "byte-identical to 모험가배낭/모험가배낭.spr" alone misses ~260 of
// the bad slots). Anchoring on the count rather than on those 8 hashes keeps the
// rule readable and lets it survive a client patch that redraws the bag.
const ROBE_TEMPLATE_MIN_FOLDERS = 10;

// The per-job .spr contents that are template leftovers. `perFolder` maps a robe
// folder name to its per-job entries ({ hash }); a content is a leftover when at
// least `minFolders` distinct folders carry it. Returns a Set of hashes.
export function robeTemplateHashes(perFolder, minFolders = ROBE_TEMPLATE_MIN_FOLDERS) {
  const owners = new Map(); // hash -> Set(folder)
  for (const [folder, entries] of perFolder) {
    for (const { hash } of entries) {
      if (!owners.has(hash)) owners.set(hash, new Set());
      owners.get(hash).add(folder);
    }
  }
  const out = new Set();
  for (const [hash, folders] of owners) if (folders.size >= minFolders) out.add(hash);
  return out;
}

// Index a robe tree: folder name -> [{ path, hash }] for its per-job sprites.
// Only .spr files inside a gender directory count. The folder-root sprite
// (로브/<g>/<g>.spr, and the nested 로브/<g>/<g>/<g>.spr) is the image bank the
// leftovers have to fall back to, so it is never a candidate — it lives directly
// in the garment folder, never under 남/여, which is what keeps it out.
function indexRobeSprites(robeRoot) {
  const perFolder = new Map();
  if (!existsSync(robeRoot)) return perFolder;
  for (const folder of readdirSync(robeRoot, { withFileTypes: true })) {
    if (!folder.isDirectory()) continue;
    const entries = [];
    perFolder.set(folder.name, entries);
    const dir = join(robeRoot, folder.name);
    // Gender dirs sit either directly in the folder (classic layout) or one level
    // down under a repeated folder name (nested layout, e.g. c_rata_tail).
    const genderDirs = [dir, join(dir, folder.name)].flatMap((base) =>
      kGENDER_DIRS.map((g) => join(base, g)),
    );
    for (const gdir of genderDirs) {
      if (!existsSync(gdir)) continue;
      for (const f of readdirSync(gdir)) {
        if (!f.toLowerCase().endsWith(".spr")) continue;
        const path = join(gdir, f);
        entries.push({ path, hash: createHash("md5").update(readFileSync(path)).digest("hex") });
      }
    }
  }
  return perFolder;
}

// robeRootSpriteHash returns the md5 of the folder's root .spr — its own artwork
// — in either the classic or the nested layout, or null when it ships none.
// Captured into the index so the prune decision needs no filesystem.
function robeRootSpriteHash(robeRoot, folder) {
  const dir = join(robeRoot, folder);
  for (const p of [join(dir, `${folder}.spr`), join(dir, folder, `${folder}.spr`)]) {
    if (existsSync(p)) return createHash("md5").update(readFileSync(p)).digest("hex");
  }
  return null;
}

// ---------------------------------------------------------------------------
// The robe index: the prune decision, separated from the pruning.
//
// Identifying a leftover requires comparing file *contents* across folders, and
// the robe tree is 134,707 sprites / ~4.5 GB — three quarters of the whole
// sprite library. Hashing it means hydrating all of it, which a CI runner with
// 14 GB of disk cannot do casually and does not need to: the decision only ever
// looks at hashes, never at bytes.
//
// So the index is the durable artifact. The upload step already reads every file
// it ships and can record path -> md5 as it goes; from then on pruning is pure
// arithmetic over that record. It also means the pipeline can compute which
// object keys to delete without the objects being anywhere near it.
//
// ROBE_INDEX_VERSION guards the shape. Bump it if the schema changes, so a stale
// index is refused rather than silently producing a wrong prune list:
// robePrunePlan throws on one, and --prune-robes --index rebuilds it from the tree.
// ---------------------------------------------------------------------------

// v2 added rootHash, which the sibling-copy rule needs.
export const ROBE_INDEX_VERSION = 2;

/**
 * buildRobeIndex(resourcesDir) reads the robe tree and records what pruning
 * needs: every per-job sprite's hash, and the folder's root sprite hash.
 *
 * Paths are forward-slashed and relative to resourcesDir, so they map straight
 * onto object keys as well as onto the tree.
 */
export function buildRobeIndex(resourcesDir) {
  const root = resolve(resourcesDir);
  const robeRoot = join(root, "data", "sprite", kROBE);
  const folders = {};
  for (const [folder, entries] of indexRobeSprites(robeRoot)) {
    const rootHash = robeRootSpriteHash(robeRoot, folder);
    folders[folder] = {
      // Filesystem probes, captured once so the decision below needs no
      // filesystem at all.
      root: rootHash !== null,
      rootHash,
      sprites: entries.map((e) => ({
        path: relPosix(root, e.path),
        hash: e.hash,
      })),
    };
  }
  return { version: ROBE_INDEX_VERSION, minFolders: ROBE_TEMPLATE_MIN_FOLDERS, folders };
}

/**
 * robePrunePlan(index) decides what to remove. Pure — no filesystem, no bytes.
 *
 * Returns { remove, folders, emptied }: the paths to delete, a per-folder
 * summary, and the folders left with no sprite at all.
 */
export function robePrunePlan(index) {
  if (index?.version !== ROBE_INDEX_VERSION) {
    throw new Error(
      `robe index version ${index?.version}, expected ${ROBE_INDEX_VERSION} — rebuild it`,
    );
  }
  const minFolders = index.minFolders ?? ROBE_TEMPLATE_MIN_FOLDERS;

  // robeTemplateHashes wants Map<folder, [{hash}]>, which is what it already
  // gets from the filesystem walk; feed it the recorded hashes instead.
  const perFolder = new Map(
    Object.entries(index.folders).map(([folder, f]) => [folder, f.sprites]),
  );
  const leftovers = robeTemplateHashes(perFolder, minFolders);
  const copies = robeSiblingCopies(index.folders);

  const remove = [];
  const folders = [];
  const emptied = [];
  for (const [folder, f] of Object.entries(index.folders)) {
    const copy = copies.get(folder);
    const hits = f.sprites.filter((s) => leftovers.has(s.hash) || copy?.hashes.has(s.hash));
    if (!hits.length) continue;
    const survivors = f.sprites.length - hits.length;
    folders.push({ folder, removed: hits.length, survivors, copiedFrom: copy ? [...copy.from] : [] });
    if (survivors === 0 && !f.root) emptied.push(folder);
    for (const h of hits) remove.push(h.path);
  }
  folders.sort((a, b) => b.removed - a.removed);
  return { remove, folders, emptied, leftoverHashes: leftovers.size };
}

// robeSiblingCopies finds robe folders Gravity built by copying ANOTHER garment's
// folder rather than the backpack template — the same mistake, with a donor too
// rare for the ≥10-folder rule to see. c_pitaya_r_bag ([Visual] Cesta de Pitaya
// Vermelha, view 245) is c_pitaya_g_bag's folder with a red root .spr dropped in:
// 343 of its 355 per-job sprites are the green basket, so every job but the three
// new 4th-class bodies rendered green. huse_luk_r_wing (Asas de Garuda, 160) is
// angelribbonwing's the same way.
//
// The evidence is exact rather than statistical: folder F has its own root .spr,
// and one of F's per-job slots is byte-identical to a DIFFERENT garment G's root
// .spr. That slot cannot be F's artwork — F's artwork is its root — so F was
// copied from G, and every per-job sprite in F whose content G also carries (G's
// root or G's per-job bank) is G's leftover. Folders that share one root .spr
// (c_g_daehyon_sword_tw / c_t_bear_bag / c_valkyrie_wing) are the same artwork and
// never count as donors to each other. A folder with no root .spr is skipped: its
// per-job sprites are all the artwork it has, so there is nothing to fall back to.
//
// Returns Map<folder, { from: Set<donor>, hashes: Set<hash> }>.
export function robeSiblingCopies(folders) {
  const rootOwners = new Map(); // root hash -> [folder]
  for (const [folder, f] of Object.entries(folders)) {
    if (!f.rootHash) continue;
    if (!rootOwners.has(f.rootHash)) rootOwners.set(f.rootHash, []);
    rootOwners.get(f.rootHash).push(folder);
  }
  const out = new Map();
  for (const [folder, f] of Object.entries(folders)) {
    if (!f.rootHash) continue;
    for (const { hash } of f.sprites) {
      if (hash === f.rootHash) continue;
      for (const donor of rootOwners.get(hash) ?? []) {
        if (!out.has(folder)) out.set(folder, { from: new Set(), hashes: new Set() });
        const copy = out.get(folder);
        if (copy.from.has(donor)) continue;
        copy.from.add(donor);
        copy.hashes.add(hash);
        for (const s of folders[donor].sprites) copy.hashes.add(s.hash);
      }
    }
    // The folder's own artwork is never a leftover, even if a donor carries it.
    out.get(folder)?.hashes.delete(f.rootHash);
  }
  return out;
}

// relPosix renders an absolute path relative to root with forward slashes.
function relPosix(root, p) {
  return p.slice(root.length).replace(/^[\\/]+/, "").split("\\").join("/");
}

function pruneRobes(resourcesDir, { dryRun = false, indexPath = null } = {}) {
  const robeRoot = join(resolve(resourcesDir), "data", "sprite", kROBE);
  if (!existsSync(robeRoot)) {
    console.error(`No robe tree at ${robeRoot} — run --extract first.`);
    process.exitCode = 1;
    return;
  }

  let index;
  if (indexPath) {
    console.error(`Reading robe index ${indexPath}…`);
    try {
      index = JSON.parse(readFileSync(indexPath, "utf8"));
    } catch (e) {
      console.error(`  unreadable (${e.message})`);
      index = null;
    }
    // The patch cycle keeps this index across deploys, so a schema bump — or a
    // write cut short — would otherwise fail every patch until someone deleted
    // the file by hand. The tree it describes is right here, so rebuild it.
    if (index?.version !== ROBE_INDEX_VERSION) {
      console.error(`  index is version ${index?.version}, expected ${ROBE_INDEX_VERSION} — rebuilding`);
      index = buildRobeIndex(resourcesDir);
      if (!dryRun) writeFileSync(indexPath, JSON.stringify(index));
    }
  } else {
    console.error(`Indexing ${robeRoot}…`);
    index = buildRobeIndex(resourcesDir);
  }

  const folderCount = Object.keys(index.folders).length;
  const total = Object.values(index.folders).reduce((n, f) => n + f.sprites.length, 0);
  const plan = robePrunePlan(index);
  console.error(
    `  ${folderCount} folders, ${total} per-job sprites, ` +
      `${plan.leftoverHashes} template content(s) shared by ≥${index.minFolders ?? ROBE_TEMPLATE_MIN_FOLDERS} folders`,
  );

  let removed = 0;
  const root = resolve(resourcesDir);
  for (const rel of plan.remove) {
    if (!dryRun) rmSync(join(root, rel), { force: true });
    removed++;
  }

  const rows = plan.folders;
  const emptied = plan.emptied;
  for (const r of rows) {
    const from = r.copiedFrom.length ? `  (copy of ${r.copiedFrom.join(", ")})` : "";
    console.error(`  ${r.folder.padEnd(26)} ${String(r.removed).padStart(4)} removed, ${r.survivors} kept${from}`);
  }
  console.error(
    `\n${dryRun ? "Would remove" : "Removed"} ${removed} template leftover(s) from ${rows.length} folder(s).`,
  );
  // A folder with nothing left has no artwork anywhere in the client, so its
  // costume is not a sprite at all — it is a .str world effect --effects should
  // be picking up. c_snow_powder is the one such folder today (see
  // GARMENT_TEMPLATE_ONLY); anything new showing up here needs the same look.
  if (emptied.length) {
    console.error(
      `! ${emptied.length} folder(s) now have no sprite at all: ${emptied.join(", ")}\n` +
        `  Those garments ship no artwork — check whether they are .str effects ` +
        `(GARMENT_TEMPLATE_ONLY / --effects).`,
    );
  }
}

// ---------------------------------------------------------------------------
// Minimal Lua 5.1 bytecode VM — just enough to execute the Ragnarok client's
// data-table chunks (System/iteminfo_new.lub, skillid.lub, etc.). These files
// are pure table constructors assigned to globals: no loops, branches, or
// arithmetic, so we only implement the opcodes they actually use and throw on
// anything unexpected. Run a chunk and read the resulting globals.
//
// String constants are kept as latin1 (1:1 byte<->codepoint) so the caller can
// re-decode the original bytes with the right charset (client data mixes CP1252
// Portuguese with EUC-KR Korean). See decodeClientString().
//
// Inlined from adsonpleal/ragreplaystats (tools/lua51.mjs).
// ---------------------------------------------------------------------------

// Lua 5.1 opcode numbers (lopcodes.h order).
// prettier-ignore
const OP = {
  MOVE: 0, LOADK: 1, LOADBOOL: 2, LOADNIL: 3, GETUPVAL: 4, GETGLOBAL: 5,
  GETTABLE: 6, SETGLOBAL: 7, SETUPVAL: 8, SETTABLE: 9, NEWTABLE: 10, SELF: 11,
  ADD: 12, SUB: 13, MUL: 14, DIV: 15, MOD: 16, POW: 17, UNM: 18, NOT: 19,
  LEN: 20, CONCAT: 21, JMP: 22, EQ: 23, LT: 24, LE: 25, TEST: 26, TESTSET: 27,
  CALL: 28, TAILCALL: 29, RETURN: 30, FORLOOP: 31, FORPREP: 32, TFORLOOP: 33,
  SETLIST: 34, CLOSE: 35, CLOSURE: 36, VARARG: 37,
};
const FIELDS_PER_FLUSH = 50;
const BITRK = 1 << 8;

// Exported so the --raw projections can be unit-tested against a hand-built
// table instead of needing a 4.3 GB GRF.
export class LuaTable {
  constructor() {
    this.map = new Map();
  }
  set(k, v) {
    if (v === undefined || v === null) this.map.delete(k);
    else this.map.set(k, v);
  }
  get(k) {
    return this.map.get(k);
  }
}

function loadChunk(bytes) {
  const buf = Buffer.from(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  if (buf[0] !== 0x1b || buf[1] !== 0x4c || buf[2] !== 0x75 || buf[3] !== 0x61)
    throw new Error("not a Lua chunk");
  if (buf[4] !== 0x51) throw new Error(`unsupported Lua version 0x${buf[4].toString(16)}`);
  const c = {
    buf,
    pos: 12,
    sizeofInt: buf[7],
    sizeofSizeT: buf[8],
    sizeofInstr: buf[9],
    sizeofNumber: buf[10],
  };
  if (c.sizeofInstr !== 4) throw new Error("only 4-byte instructions supported");
  return readProto(c);
}

function readUInt(c, n) {
  let val = 0;
  for (let i = 0; i < n; i++) val += c.buf[c.pos + i] * 2 ** (8 * i);
  c.pos += n;
  return val;
}

function readString(c) {
  const len = readUInt(c, c.sizeofSizeT);
  if (len === 0) return null;
  const start = c.pos;
  c.pos += len;
  return c.buf.toString("latin1", start, start + len - 1); // drop trailing \0
}

function readProto(c) {
  readString(c); // source name
  c.pos += c.sizeofInt; // line defined
  c.pos += c.sizeofInt; // last line defined
  c.pos += 4; // nups, numparams, is_vararg, maxstacksize

  const sizecode = readUInt(c, c.sizeofInt);
  const code = new Array(sizecode);
  for (let i = 0; i < sizecode; i++) {
    code[i] = c.buf.readUInt32LE(c.pos);
    c.pos += 4;
  }

  const sizek = readUInt(c, c.sizeofInt);
  const k = new Array(sizek);
  for (let i = 0; i < sizek; i++) {
    const type = c.buf[c.pos++];
    if (type === 0) k[i] = undefined;
    else if (type === 1) k[i] = c.buf[c.pos++] !== 0;
    else if (type === 3) {
      k[i] = c.buf.readDoubleLE(c.pos);
      c.pos += 8;
    } else if (type === 4) k[i] = readString(c);
    else throw new Error(`unknown constant type ${type}`);
  }

  const sizep = readUInt(c, c.sizeofInt);
  const protos = new Array(sizep);
  for (let i = 0; i < sizep; i++) protos[i] = readProto(c);

  // debug blocks — skip
  const lineInfo = readUInt(c, c.sizeofInt);
  c.pos += lineInfo * c.sizeofInt;
  const locals = readUInt(c, c.sizeofInt);
  for (let i = 0; i < locals; i++) {
    readString(c);
    c.pos += c.sizeofInt * 2;
  }
  const upvals = readUInt(c, c.sizeofInt);
  for (let i = 0; i < upvals; i++) readString(c);

  return { code, k, protos };
}

// Executes a single proto over a shared globals table.
function execute(proto, globals) {
  const R = [];
  const K = proto.k;
  const rk = (x) => (x & BITRK ? K[x & (BITRK - 1)] : R[x]);
  let pc = 0;
  while (pc < proto.code.length) {
    const i = proto.code[pc++];
    const op = i & 0x3f;
    const a = (i >>> 6) & 0xff;
    const c = (i >>> 14) & 0x1ff;
    const b = (i >>> 23) & 0x1ff;
    const bx = (i >>> 14) & 0x3ffff;

    switch (op) {
      case OP.MOVE: R[a] = R[b]; break;
      case OP.LOADK: R[a] = K[bx]; break;
      case OP.LOADBOOL: R[a] = b !== 0; if (c) pc++; break;
      case OP.LOADNIL: for (let r = a; r <= b; r++) R[r] = undefined; break;
      case OP.GETGLOBAL: R[a] = globals.get(K[bx]); break;
      case OP.SETGLOBAL: globals.set(K[bx], R[a]); break;
      case OP.NEWTABLE: R[a] = new LuaTable(); break;
      case OP.GETTABLE: {
        const t = R[b];
        R[a] = t instanceof LuaTable ? t.get(rk(c)) : undefined;
        break;
      }
      case OP.SETTABLE: {
        const t = R[a];
        if (t instanceof LuaTable) t.set(rk(b), rk(c));
        break;
      }
      case OP.SETLIST: {
        let n = b;
        let block = c;
        if (block === 0) block = proto.code[pc++]; // real C in next word
        if (n === 0) throw new Error("SETLIST with B=0 (vararg) not supported");
        const base = (block - 1) * FIELDS_PER_FLUSH;
        const t = R[a];
        for (let j = 1; j <= n; j++) t.set(base + j, R[a + j]);
        break;
      }
      case OP.CLOSURE: {
        // Represent nested closures as their proto; calling is a no-op below.
        R[a] = { __proto_index: bx, proto: proto.protos[bx] };
        // CLOSURE is followed by `nups` pseudo-instructions (MOVE/GETUPVAL);
        // skip them so we don't misread them as real ops.
        // We don't track nups here, but data chunks have no upvalue captures
        // on these closures, so there is nothing to skip in practice.
        break;
      }
      case OP.CALL: break; // ignore calls — data chunks build tables, not effects
      case OP.TAILCALL: break;
      case OP.RETURN: return; // end of chunk
      // Every data table but one is straight-line code. SkillInfoList_data.lub
      // ends with a guarded `for k,v in pairs(...)` that folds the localized
      // names in, so the three ops that shape it are real here: the jump moves
      // the pc (sBx is biased by MAXARG_sBx), and TEST/TFORLOOP each decide
      // whether the jump that follows them runs at all.
      case OP.JMP: pc += bx - 131071; break;
      case OP.TEST: {
        // lvm.c: jump when `l_isfalse(RA) != C`; otherwise step over the jump.
        // Lua truthiness, not JS — 0 and "" are true, only nil and false are not.
        const isFalse = R[a] === undefined || R[a] === false;
        if ((isFalse ? 1 : 0) === c) pc++;
        break;
      }
      // A generic for is driven by calling its iterator, and CALL is a no-op
      // here, so there is never a first value to loop over: skip the jump back
      // into the body and let the loop end. This is why running that chunk
      // leaves the copies those loops would have made unmade — read the table
      // the chunk *builds*, not the one it merges into.
      case OP.TFORLOOP: pc++; break;
      default:
        throw new Error(`unimplemented opcode ${op} at pc ${pc - 1}`);
    }
  }
}

// Run a Lua 5.1 chunk over an existing globals table (so dependent chunks can
// share state); runChunk starts from a fresh one.
export function runChunkInto(bytes, globals) {
  execute(loadChunk(bytes), globals);
  return globals;
}

// Exported for the VM's own tests: the branch ops above are the one part of it
// whose behaviour can't be read off a projected table.
export function runChunk(bytes) {
  return runChunkInto(bytes, new LuaTable());
}

// Client strings are CP1252 (Portuguese) or EUC-KR (Korean, untranslated). The
// VM keeps them as latin1, so recover the bytes and pick the charset: prefer a
// clean EUC-KR decode that yields Hangul, else fall back to Windows-1252.
// (The two charsets overlap — see the tie-break inside.)
const UTF8 = new TextDecoder("utf-8", { fatal: true });
const EUCKR = new TextDecoder("euc-kr", { fatal: true });
const CP1252 = new TextDecoder("windows-1252");
const HANGUL = /[\uac00-\ud7af]/;
// Whether a string reads as plausible accented Latin text rather than another
// charset misread as one: every non-ASCII character is a letter, and no three
// of them run together (Portuguese tops out at two, "AÇÃO"; a Korean name
// misread as CP1252 is two characters per syllable, so four and up).
const isLatinText = (s) =>
  [...s].every((ch) => ch.charCodeAt(0) < 0x80 || /\p{L}/u.test(ch)) &&
  !/[\u0080-\uffff]{3}/.test(s);
// LATAM accent repair. Every Portuguese accent in a handful of item strings
// arrives as a Cyrillic letter out of KS X 1001 row 12 (lead byte 0xAC):
// "Cabeуa" for "Cabeça", "Nьvel necessрrio" for "Nível necessário". EUC-KR
// encodes no accented Latin at all — the whole A1-FE x A1-FE space contains not
// one of á à â ã é ê í ó ô õ ú ç — so upstream of the client the translators'
// text was read through a plain EUC-KR table and re-encoded from the Cyrillic
// that came out. What reaches us is therefore UTF-8-encoded Cyrillic, not
// row-0xAC bytes: the strings sail through the UTF-8 branch below intact and
// wrong, which is why this repair runs on decoded text instead of inside the
// charset ladder.
//
// Only these six cells are attested, and the mapping is arbitrary — é is cell
// D8 and ê is A8, nowhere near their Latin 0xE9/0xEA. The bundled client fonts
// (System/font/SCDream{4,6}.otf) carry Cyrillic and Hangul but no accented Latin
// whatsoever, so there is no glyph table to derive the other 60 cells from.
// Guessing them would invent a mapping nothing supports, so an unmapped Cyrillic
// character is left alone and reported at write time instead.
const LATAM_ACCENTS = new Map([
  ["\u0416", "\u00ea"], // AC A8 -> ê   (Cyrillic capital Zhe)
  ["\u0436", "\u00e9"], // AC D8 -> é   (Cyrillic small zhe)
  ["\u0440", "\u00e1"], // AC E2 -> á   (Cyrillic small er)
  ["\u0441", "\u00e3"], // AC E3 -> ã   (Cyrillic small es)
  ["\u0443", "\u00e7"], // AC E5 -> ç   (Cyrillic small u)
  ["\u044c", "\u00ed"], // AC EE -> í   (Cyrillic soft sign)
]);
const CYRILLIC = /[\u0400-\u04ff]/;
// Repair only Latin text with Cyrillic sprinkled through it. A string carrying a
// Cyrillic letter outside the table is either genuine Cyrillic or a row-0xAC
// cell nobody has pinned down yet; substituting just the known ones would leave
// a half-converted mess either way, so hand it back whole to be reported.
function repairLatamAccents(s) {
  if (!CYRILLIC.test(s) || !/[A-Za-z]/.test(s)) return s;
  if ([...s].some((ch) => CYRILLIC.test(ch) && !LATAM_ACCENTS.has(ch))) return s;
  return s.replace(/[\u0400-\u04ff]/g, (ch) => LATAM_ACCENTS.get(ch));
}

export function decodeClientString(latin1) {
  const decoded = decodeClientCharset(latin1);
  return decoded == null ? decoded : repairLatamAccents(decoded);
}

function decodeClientCharset(latin1) {
  if (latin1 == null) return null;
  const bytes = Buffer.from(latin1, "latin1");
  if (!bytes.some((x) => x >= 0x80)) return latin1; // pure ASCII
  // The patched iteminfo_new.lub is UTF-8; a strict decode succeeds only for
  // genuine UTF-8 and cleanly covers both Portuguese and Korean. Legacy strings
  // fall back: EUC-KR when it decodes to Korean, else CP1252.
  try {
    return UTF8.decode(bytes);
  } catch {
    /* not UTF-8 */
  }
  let korean = null;
  try {
    korean = EUCKR.decode(bytes);
  } catch {
    /* not EUC-KR either */
  }
  if (korean != null) {
    // Nothing Latin to disagree with: the legacy pure-Korean case, which may be
    // punctuation- or Hanja-only, so any clean decode wins.
    if (!/[A-Za-z]/.test(latin1)) return korean;
    // Otherwise the two readings compete. ASCII + Hangul is a real combination —
    // the name tables prefix Korean sprite names with ASCII ("_C홍염의폭렬파동"
    // in AccNameTable) — but a strict EUC-KR decode succeeding proves nothing on
    // its own: an uppercase Portuguese pair like "ÇÃ" is a valid Hangul double
    // byte too, so "AÇÃO" would "decode" to Korean. What tells them apart is the
    // other reading: real accented text is letters all the way through and never
    // stacks three in a row, while EUC-KR bytes read as CP1252 spill long runs of
    // symbols ("_C홍염의폭렬파동" comes out as "_CÈ«¿°ÀÇÆø·ÄÆÄµ¿"). So take the
    // Korean only when the CP1252 reading is not plausible text.
    const latin = CP1252.decode(bytes);
    if (HANGUL.test(korean) && !isLatinText(latin)) return korean;
    return latin;
  }
  return CP1252.decode(bytes);
}

// ---------------------------------------------------------------------------
// Icon id mapping — items come from System/iteminfo_new.lub (a sibling of
// data.grf), skills from skillid.lub inside the GRF.
// ---------------------------------------------------------------------------

// Allow an explicit override via --iteminfo; otherwise look next to the GRF.
function resolveItemInfoPath(args) {
  if (args.iteminfo) return existsSync(args.iteminfo) ? args.iteminfo : null;
  const root = join(dirname(resolve(args.grf)), "System");
  for (const name of ["iteminfo_new.lub", "itemInfo.lub", "iteminfo.lub"]) {
    const p = join(root, name);
    // Skip the tiny stub itemInfo.lub (a few hundred bytes that just chains
    // to the real table).
    if (existsSync(p) && statSync(p).size > 4096) return p;
  }
  return null;
}

// id -> icon resource name (lowercased). The live System/iteminfo_new.lub is
// authoritative and complete (modern equipment like 450147 = "Illusion_Armor_A"
// is only there).
function buildResNameMap(args) {
  const out = new Map();
  const lubPath = resolveItemInfoPath(args);
  if (!lubPath) {
    throw new Error(
      "iteminfo_new.lub not found next to the GRF (System/) — pass --iteminfo <path>",
    );
  }
  const tbl = runChunk(readFileSync(lubPath)).get("tbl");
  if (tbl instanceof LuaTable) {
    for (const [id, entry] of tbl.map) {
      if (typeof id !== "number" || !(entry instanceof LuaTable)) continue;
      const res =
        decodeClientString(entry.get("identifiedResourceName")) ||
        decodeClientString(entry.get("unidentifiedResourceName"));
      if (res) out.set(String(id), res.toLowerCase());
    }
  }
  return out;
}

// SKID const -> numeric id, from executing skillid.lub (it defines the SKID
// table). Skill icons live in the item folder named after the lowercased
// const (e.g. SKID.AL_HEAL = 28 -> item/al_heal.bmp -> skill/28.png).
function parseSkillIds(map) {
  const ids = new Map();
  const bytes =
    map.get("data/luafiles514/lua files/skillinfoz/skillid.lub") ??
    map.get("data/luafiles514/lua files/skillinfoz/skillid.lua");
  if (!bytes) return ids;
  try {
    return skidMap(runChunk(bytes).get("SKID"));
  } catch (err) {
    console.error(`! skillid.lub could not be executed (${err.message}); skipping skill icons`);
  }
  return ids;
}

// The SKID table as const -> id.
export function skidMap(tbl) {
  const ids = new Map();
  if (!(tbl instanceof LuaTable)) return ids;
  for (const [konst, id] of tbl.map) {
    if (typeof konst === "string" && typeof id === "number") ids.set(konst, Math.round(id));
  }
  return ids;
}

// Supplemental EFST id -> icon filename table for status effects the client
// shows an icon for but that StateIconImgList (stateiconimginfo.lub) never
// references. For these ids the client maps the EFST to a data/texture/effect/
// *.tga via a convention hardcoded in the client exe, not the lua data, so they
// would otherwise 404. Values are basenames relative to data/texture/effect/
// (same convention as StateIconImgList entries), resolved against the GRF by the
// writer. StateIconImgList wins whenever it has its own entry for an id (see
// parseStatusIcons). Note these TGAs are not necessarily 32x32 — the *_gogi
// stat-food icons are 32x24 — so the writer must read each TGA's own header.
const STATUS_ICON_OVERRIDES = {
  // Stat food buffs (EFST_FOOD_*) and their cash-shop variants (EFST_FOOD_*_CASH).
  241: "str_gogi.tga", // EFST_FOOD_STR
  242: "agi_gogi.tga", // EFST_FOOD_AGI
  243: "vit_gogi.tga", // EFST_FOOD_VIT
  244: "dex_gogi.tga", // EFST_FOOD_DEX
  245: "int_gogi.tga", // EFST_FOOD_INT
  246: "luk_gogi.tga", // EFST_FOOD_LUK
  271: "str_gogi.tga", // EFST_FOOD_STR_CASH
  272: "agi_gogi.tga", // EFST_FOOD_AGI_CASH
  273: "vit_gogi.tga", // EFST_FOOD_VIT_CASH
  274: "dex_gogi.tga", // EFST_FOOD_DEX_CASH
  275: "int_gogi.tga", // EFST_FOOD_INT_CASH
  276: "luk_gogi.tga", // EFST_FOOD_LUK_CASH
};

// EFST status-effect id -> icon filename. efstids.lub defines the global
// EFST_IDs (name -> numeric id); stateiconimginfo.lub then builds
// StateIconImgList[priority][EFST_IDs[name]] = "<file>.tga", so it must run over
// the SAME globals AFTER efstids for those lookups to resolve. We flatten the
// per-priority sub-tables down to id -> filename (the filename is a client
// string — EUC-KR for the Korean names — resolved against the GRF later), then
// fill in ids the lua table omits from STATUS_ICON_OVERRIDES (lua data wins).
function parseStatusIcons(map) {
  const out = new Map();
  const efst = map.get("data/luafiles514/lua files/stateicon/efstids.lub");
  const img = map.get("data/luafiles514/lua files/stateicon/stateiconimginfo.lub");
  if (!efst || !img) {
    console.error("! stateicon lua tables not found in GRF; skipping status icons");
    return out;
  }
  try {
    const globals = new LuaTable();
    runChunkInto(efst, globals);
    runChunkInto(img, globals);
    const list = globals.get("StateIconImgList");
    if (list instanceof LuaTable) {
      for (const [, sub] of list.map) {
        if (!(sub instanceof LuaTable)) continue;
        for (const [id, file] of sub.map) {
          if (typeof id === "number" && typeof file === "string" && file) out.set(id, file);
        }
      }
    }
  } catch (err) {
    console.error(`! stateicon tables could not be executed (${err.message}); skipping status icons`);
  }
  // Apply the hardcoded override table for ids StateIconImgList doesn't cover.
  for (const [id, file] of Object.entries(STATUS_ICON_OVERRIDES)) {
    const n = Number(id);
    if (!out.has(n)) out.set(n, file);
  }
  return out;
}

// ---------------------------------------------------------------------------
// BMP -> PNG conversion. RO icons are uncompressed BMPs (8-bit palettized,
// some 24/32-bit) that use magenta #FF00FF as the transparency colorkey. We
// decode to RGBA (keying magenta -> alpha 0) and re-encode as a PNG using only
// node:zlib — no external image library. Some char-creation UI elements use a
// solid corner background instead of magenta; those are keyed separately via
// keyCornerBackground (see bmpToPng's keyCorners option).
// ---------------------------------------------------------------------------

function bmpToRgba(buf) {
  const b = Buffer.from(buf.buffer, buf.byteOffset, buf.byteLength);
  if (b.length < 54 || b[0] !== 0x42 || b[1] !== 0x4d) return null; // "BM"
  const dataOffset = b.readUInt32LE(10);
  const dibSize = b.readUInt32LE(14);
  const w = b.readInt32LE(18);
  const rawH = b.readInt32LE(22);
  const bpp = b.readUInt16LE(28);
  const compression = b.readUInt32LE(30);
  if (compression !== 0 || w <= 0 || rawH === 0) return null; // BI_RGB only
  const topDown = rawH < 0;
  const h = Math.abs(rawH);

  let palette = null;
  if (bpp <= 8) {
    let palCount = b.readUInt32LE(46); // biClrUsed
    if (!palCount) palCount = 1 << bpp;
    const palStart = 14 + dibSize;
    palette = new Array(palCount);
    for (let i = 0; i < palCount; i++) {
      const o = palStart + i * 4; // stored BGRA
      palette[i] = [b[o + 2], b[o + 1], b[o]];
    }
  } else if (bpp !== 24 && bpp !== 32) {
    return null; // unsupported depth
  }

  const rowSize = Math.floor((bpp * w + 31) / 32) * 4; // padded to 4 bytes
  const rgba = Buffer.alloc(w * h * 4);
  let magenta = 0; // count of colorkeyed pixels (used to pick the alpha strategy)
  for (let row = 0; row < h; row++) {
    const srcRow = topDown ? row : h - 1 - row; // BMP rows are bottom-up
    const srcBase = dataOffset + srcRow * rowSize;
    for (let x = 0; x < w; x++) {
      let r, g, bl;
      if (bpp === 8) {
        const p = palette[b[srcBase + x]] || [0, 0, 0];
        [r, g, bl] = p;
      } else if (bpp === 4) {
        const byte = b[srcBase + (x >> 1)];
        const p = palette[x & 1 ? byte & 0x0f : byte >> 4] || [0, 0, 0];
        [r, g, bl] = p;
      } else if (bpp === 1) {
        const byte = b[srcBase + (x >> 3)];
        const p = palette[(byte >> (7 - (x & 7))) & 1] || [0, 0, 0];
        [r, g, bl] = p;
      } else if (bpp === 24) {
        const o = srcBase + x * 3;
        bl = b[o]; g = b[o + 1]; r = b[o + 2];
      } else {
        const o = srcBase + x * 4; // 32bpp BGRA — ignore stored alpha
        bl = b[o]; g = b[o + 1]; r = b[o + 2];
      }
      const di = (row * w + x) * 4;
      rgba[di] = r;
      rgba[di + 1] = g;
      rgba[di + 2] = bl;
      const isMagenta = r === 255 && g === 0 && bl === 255;
      if (isMagenta) magenta++;
      rgba[di + 3] = isMagenta ? 0 : 255; // magenta key
    }
  }
  return { width: w, height: h, rgba, magenta };
}

// Some bitmaps don't use the magenta colorkey at all but sit on a solid
// background that fills the area outside their (rounded) artwork — e.g. the
// character-creation gender/arrow buttons, whose corners are a pale pink/grey
// rather than #FF00FF. Make that background transparent by flood-filling inward
// from the four corners: each corner seeds its own colour and connected pixels
// within TOL of that seed are keyed. Connectivity (vs. a global colour match)
// keeps interior pixels that merely happen to share the corner colour. If the
// fill would swallow most of the image the corner colour was the artwork's own
// fill (e.g. the white close button), so it's reverted and left opaque.
const CORNER_TOL = 24; // max per-channel Manhattan distance from the corner seed
const CORNER_GUARD = 0.5; // abort the fill if it would key >= this fraction

function keyCornerBackground(width, height, rgba) {
  const seen = new Uint8Array(width * height);
  const keyed = [];
  const stack = [];
  for (const [sx, sy] of [[0, 0], [width - 1, 0], [0, height - 1], [width - 1, height - 1]]) {
    const si = (sy * width + sx) * 4;
    stack.push([sx, sy, rgba[si], rgba[si + 1], rgba[si + 2]]);
  }
  while (stack.length) {
    const [x, y, sr, sg, sb] = stack.pop();
    if (x < 0 || y < 0 || x >= width || y >= height) continue;
    const idx = y * width + x;
    if (seen[idx]) continue;
    const o = idx * 4;
    if (Math.abs(rgba[o] - sr) + Math.abs(rgba[o + 1] - sg) + Math.abs(rgba[o + 2] - sb) > CORNER_TOL)
      continue;
    seen[idx] = 1;
    rgba[o + 3] = 0;
    keyed.push(o);
    stack.push([x + 1, y, sr, sg, sb], [x - 1, y, sr, sg, sb], [x, y + 1, sr, sg, sb], [x, y - 1, sr, sg, sb]);
  }
  if (keyed.length >= width * height * CORNER_GUARD) {
    for (const o of keyed) rgba[o + 3] = 255; // not a background border — revert
  }
}

const PNG_CRC = (() => {
  const t = new Uint32Array(256);
  for (let n = 0; n < 256; n++) {
    let c = n;
    for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1;
    t[n] = c >>> 0;
  }
  return t;
})();

function crc32(buf) {
  let c = 0xffffffff;
  for (let i = 0; i < buf.length; i++) c = PNG_CRC[(c ^ buf[i]) & 0xff] ^ (c >>> 8);
  return (c ^ 0xffffffff) >>> 0;
}

function pngChunk(type, data) {
  const len = Buffer.alloc(4);
  len.writeUInt32BE(data.length, 0);
  const body = Buffer.concat([Buffer.from(type, "ascii"), data]);
  const crc = Buffer.alloc(4);
  crc.writeUInt32BE(crc32(body), 0);
  return Buffer.concat([len, body, crc]);
}

function encodePng(width, height, rgba) {
  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(width, 0);
  ihdr.writeUInt32BE(height, 4);
  ihdr[8] = 8; // bit depth
  ihdr[9] = 6; // color type RGBA
  // 10..12 = compression / filter / interlace = 0
  const stride = width * 4;
  const raw = Buffer.alloc((stride + 1) * height);
  for (let y = 0; y < height; y++) {
    raw[y * (stride + 1)] = 0; // filter: none
    rgba.copy(raw, y * (stride + 1) + 1, y * stride, y * stride + stride);
  }
  return Buffer.concat([
    Buffer.from([0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a]),
    pngChunk("IHDR", ihdr),
    pngChunk("IDAT", deflateSync(raw)),
    pngChunk("IEND", Buffer.alloc(0)),
  ]);
}

function bmpToPng(bmpBytes, opts = {}) {
  const decoded = bmpToRgba(bmpBytes);
  if (!decoded) return null;
  // For UI elements that carry no magenta colorkey, derive transparency from the
  // solid corner background instead (gender/arrow/etc. buttons).
  if (opts.keyCorners && decoded.magenta === 0) {
    keyCornerBackground(decoded.width, decoded.height, decoded.rgba);
  }
  // Full-bleed artwork (card illustrations) has no colorkey at all: undo the
  // decoder's magenta keying so a magenta pixel in the picture stays a pixel.
  if (opts.opaque) {
    for (let i = 3; i < decoded.rgba.length; i += 4) decoded.rgba[i] = 255;
  }
  return encodePng(decoded.width, decoded.height, decoded.rgba);
}

// Status (EFST) icons ship as TARGA rather than BMP. They are uncompressed
// true-colour (24/32-bit BGR(A)); 32-bit carries a real alpha channel while
// 24-bit is fully opaque. Decode to RGBA and re-use the PNG encoder. RLE
// (image type 10) is handled too, in case a future client patch uses it.
function tgaToRgba(buf) {
  const b = Buffer.from(buf.buffer, buf.byteOffset, buf.byteLength);
  if (b.length < 18) return null;
  const idLen = b[0];
  const colorMapType = b[1];
  const imageType = b[2];
  if (colorMapType !== 0 || (imageType !== 2 && imageType !== 10)) return null; // truecolor only
  const w = b.readUInt16LE(12);
  const h = b.readUInt16LE(14);
  const bpp = b[16];
  const desc = b[17];
  if (w <= 0 || h <= 0 || (bpp !== 24 && bpp !== 32)) return null;
  const bytesPP = bpp / 8;
  const topDown = (desc & 0x20) !== 0; // bit 5: 1 = top-left origin, else bottom-up
  let p = 18 + idLen; // no color map (colorMapType 0), so skip only the image id field
  const px = w * h;
  const src = Buffer.alloc(px * bytesPP); // pixels in stored (row) order
  if (imageType === 2) {
    if (p + px * bytesPP > b.length) return null;
    b.copy(src, 0, p, p + px * bytesPP);
  } else {
    let o = 0; // RLE: alternating run-length (0x80 bit) and raw packets
    while (o < src.length && p < b.length) {
      const count = (b[p++] & 0x7f) + 1;
      if (b[p - 1] & 0x80) {
        for (let i = 0; i < count && o < src.length; i++, o += bytesPP) b.copy(src, o, p, p + bytesPP);
        p += bytesPP;
      } else {
        const n = count * bytesPP;
        b.copy(src, o, p, p + n);
        o += n;
        p += n;
      }
    }
  }
  const rgba = Buffer.alloc(px * 4);
  for (let row = 0; row < h; row++) {
    const srcRow = topDown ? row : h - 1 - row;
    for (let x = 0; x < w; x++) {
      const so = (srcRow * w + x) * bytesPP;
      const di = (row * w + x) * 4;
      rgba[di] = src[so + 2]; // stored BGR(A)
      rgba[di + 1] = src[so + 1];
      rgba[di + 2] = src[so];
      rgba[di + 3] = bpp === 32 ? src[so + 3] : 255;
    }
  }
  return { width: w, height: h, rgba };
}

function tgaToPng(tgaBytes) {
  const decoded = tgaToRgba(tgaBytes);
  if (!decoded) return null;
  return encodePng(decoded.width, decoded.height, decoded.rgba);
}

// Bleed opaque colours outward into transparent pixels (alpha stays 0). The
// magenta colorkey leaves transparent texels with magenta RGB; under bilinear
// filtering / mipmaps those values bleed back in as pink fringes. Replacing each
// transparent texel's RGB with its nearest opaque neighbour's (a multi-source
// BFS) removes the halos. Used for the effect textures (the map sim filters them
// bilinearly); the icon pipeline keeps its texels as-is.
function bleedTransparent(width, height, rgba) {
  const total = width * height;
  const filled = new Uint8Array(total);
  let queue = [];
  for (let i = 0; i < total; i++) {
    if (rgba[i * 4 + 3] !== 0) {
      filled[i] = 1;
      queue.push(i);
    }
  }
  if (queue.length === 0 || queue.length === total) return; // all/none transparent
  while (queue.length) {
    const next = [];
    for (const p of queue) {
      const px = p % width;
      const po = p * 4;
      const cands = [];
      if (px > 0) cands.push(p - 1);
      if (px < width - 1) cands.push(p + 1);
      if (p - width >= 0) cands.push(p - width);
      if (p + width < total) cands.push(p + width);
      for (const q of cands) {
        if (filled[q]) continue;
        filled[q] = 1;
        const qo = q * 4;
        rgba[qo] = rgba[po];
        rgba[qo + 1] = rgba[po + 1];
        rgba[qo + 2] = rgba[po + 2]; // copy RGB only; alpha stays 0
        next.push(q);
      }
    }
    queue = next;
  }
}

// Convert a .str-referenced texture (BMP or TGA) to a transparent PNG for the map
// effect renderer. TGA keeps its 32-bit alpha (glow textures); BMP is magenta
// (#FF00FF) colorkeyed; both then get their transparent RGB bled to kill fringes.
// Returns null for unsupported encodings (caller logs + skips). Mirrors the POC
// latamvisuais/tools/bmp.mjs textureToPng so its output is byte-identical.
function effectTextureToPng(bytes, name) {
  const isTga = /\.tga$/i.test(name);
  const decoded = isTga ? tgaToRgba(bytes) : bmpToRgba(bytes);
  if (!decoded) return null;
  bleedTransparent(decoded.width, decoded.height, decoded.rgba);
  return encodePng(decoded.width, decoded.height, decoded.rgba);
}

// ---------------------------------------------------------------------------
// Icon extraction — decodes each BMP to a transparent PNG keyed by numeric id:
//   <out>/item/<id>.png        inventory icon    (item\<resname>.bmp)
//   <out>/collection/<id>.png  description image (collection\<resname>.bmp)
//   <out>/skill/<id>.png       skill icon        (item\<skid-const>.bmp)
//   <out>/job/<id>.png         class icon        (renewalparty\icon_jobs_<id>.bmp)
//   <out>/status/<id>.png      EFST status icon  (texture\effect\<file>.tga)
//   <out>/ui/<name>.png        char-creation UI  (make_character_ver2\<name>.bmp)
// resnames come from System/iteminfo_new.lub; skill icon filenames are the
// lowercased SKID constant. Magenta (#FF00FF) is mapped to transparent; UI
// elements with no magenta key instead derive transparency from their corner
// background (keyCornerBackground), which fixes the gender/arrow buttons.
// ---------------------------------------------------------------------------

const UI = "data/texture/유저인터페이스"; // "user interface" texture root

// Character-creation UI elements (gender/turn buttons, hair style thumbnails,
// hair color swatches, race images) served by basename under the `ui` kind.
const UI_DIR = `${UI}/make_character_ver2/`;

function indexIcons(grf) {
  // normalized filename -> best entry, limited to the icon folders we need.
  const idx = new Map();
  const itemDir = `${UI}/item/`;
  const collDir = `${UI}/collection/`;
  const jobPrefix = `${UI}/renewalparty/icon_jobs_`;
  for (const f of grf.files) {
    if (!(f.flags & 0x01)) continue;
    const n = normalize(f.filename);
    if (!n.endsWith(".bmp")) continue;
    if (
      !n.startsWith(itemDir) &&
      !n.startsWith(collDir) &&
      !n.startsWith(jobPrefix) &&
      !n.startsWith(UI_DIR)
    )
      continue;
    const prev = idx.get(n);
    if (!prev || f.uncompSize > prev.uncompSize) idx.set(n, f);
  }
  return idx;
}

function extractIcons(grfPath, outBase, args) {
  const grf = openGrf(grfPath);
  try {
    const root = resolve(outBase);
    const dirs = {
      item: join(root, "item"),
      collection: join(root, "collection"),
      skill: join(root, "skill"),
      job: join(root, "job"),
      status: join(root, "status"),
      ui: join(root, "ui"),
    };
    for (const d of Object.values(dirs)) mkdirSync(d, { recursive: true });

    console.error("Indexing icon entries…");
    const idx = indexIcons(grf);
    console.error(`  ${idx.size} icon files indexed`);

    const counts = { item: 0, collection: 0, skill: 0, job: 0, status: 0, ui: 0 };
    const fails = { extract: 0, convert: 0 };
    const writeIcon = (kind, id, entry) => {
      let bmp;
      try {
        bmp = extractFile(grf, entry);
      } catch {
        fails.extract++;
        return false;
      }
      const png = bmpToPng(bmp, { keyCorners: kind === "ui" });
      if (!png) {
        fails.convert++;
        return false;
      }
      writeFileSync(join(dirs[kind], `${id}.png`), png);
      counts[kind]++;
      return true;
    };

    // Item inventory + collection icons, keyed by resource name.
    const resNames = buildResNameMap(args);
    for (const [id, res] of resNames) {
      const itemEntry = idx.get(`${UI}/item/${res}.bmp`);
      if (itemEntry) writeIcon("item", id, itemEntry);
      const collEntry = idx.get(`${UI}/collection/${res}.bmp`);
      if (collEntry) writeIcon("collection", id, collEntry);
    }

    // Skill icons share the item folder, named after the lowercased SKID const.
    const fileMap = collectGrfFiles(grf, [
      "data/luafiles514/lua files/skillinfoz/skillid.lub",
      "data/luafiles514/lua files/skillinfoz/skillid.lua",
    ]);
    const skillIds = parseSkillIds(fileMap);
    const skillIcons = new Map(); // id -> the entry its icon was written from
    for (const [konst, id] of skillIds) {
      const entry = idx.get(`${UI}/item/${konst.toLowerCase()}.bmp`);
      if (entry && writeIcon("skill", id, entry)) skillIcons.set(id, entry);
    }
    // A follow-up hit the client ships no icon for is drawn with its parent's
    // (skills.json names it after the parent too).
    let borrowed = 0;
    const borrowedIds = new Set();
    for (const [id, { parentId }] of skillFollowUps(withSkidAdditions(skillIds))) {
      if (skillIcons.has(id) || !skillIcons.has(parentId)) continue;
      if (writeIcon("skill", id, skillIcons.get(parentId))) {
        borrowed++;
        borrowedIds.add(id);
      }
    }
    if (borrowed) console.error(`  ${borrowed} follow-up skill icons borrowed from their parent`);

    // The client ships no icon for any NPC_* skill, so every monster skill
    // without one gets a single generic icon — the one divine-pride shows for
    // them all, which is pixel-for-pixel the client's own S.B.R.44 icon. It is
    // never a player skill's lookalike: NPC_CHEAL gets this, not AB_CHEAL's.
    const monsterIcon = idx.get(`${UI}/item/${MONSTER_SKILL_ICON}.bmp`);
    if (!monsterIcon) {
      console.error(`  ! item/${MONSTER_SKILL_ICON}.bmp not in the GRF — monster skills get no icon`);
    } else {
      let generic = 0;
      const done = new Set();
      for (const [konst, id] of skillIds) {
        if (!konst.startsWith("NPC_") || SKID_MARKER.test(konst)) continue;
        if (skillIcons.has(id) || borrowedIds.has(id) || done.has(id)) continue;
        done.add(id);
        if (writeIcon("skill", id, monsterIcon)) generic++;
      }
      console.error(`  ${generic} monster skills drawn with the generic icon`);
    }

    // Class icons keyed directly by numeric job id (skip the _die variants).
    const jobRe = new RegExp(
      `${UI.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")}/renewalparty/icon_jobs_(\\d+)\\.bmp$`,
    );
    const jobFailed = [];
    for (const [name, entry] of idx) {
      const m = name.match(jobRe);
      if (m && !writeIcon("job", m[1], entry)) jobFailed.push(Number(m[1]));
    }

    // Status (EFST) icons: TARGA images under data/texture/effect/, keyed by the
    // numeric EFST id from the stateicon lua tables. The mapped filename is a
    // client string (EUC-KR for Korean names), decoded the same way as GRF entry
    // names so it matches the indexed path.
    const statusMap = parseStatusIcons(
      collectGrfFiles(grf, [
        "data/luafiles514/lua files/stateicon/efstids.lub",
        "data/luafiles514/lua files/stateicon/stateiconimginfo.lub",
      ]),
    );
    if (statusMap.size) {
      const effectIdx = new Map(); // normalized effect-folder path -> best entry
      for (const f of grf.files) {
        if (!(f.flags & 0x01)) continue;
        const n = normalize(f.filename);
        if (!n.startsWith("data/texture/effect/") || !n.endsWith(".tga")) continue;
        const prev = effectIdx.get(n);
        if (!prev || f.uncompSize > prev.uncompSize) effectIdx.set(n, f);
      }
      const statusMissing = [];
      for (const [id, file] of statusMap) {
        // The lua VM keeps strings as latin1; recover the raw bytes and decode
        // them the same way GRF entry names are (EUC-KR for the Korean filenames).
        const name = decodeName(Buffer.from(file, "latin1"));
        const entry = effectIdx.get(normalize(`data/texture/effect/${name}`));
        if (!entry) {
          statusMissing.push(file);
          continue;
        }
        let tga;
        try {
          tga = extractFile(grf, entry);
        } catch {
          fails.extract++;
          continue;
        }
        const png = tgaToPng(tga);
        if (!png) {
          fails.convert++;
          continue;
        }
        writeFileSync(join(dirs.status, `${id}.png`), png);
        counts.status++;
      }
      if (statusMissing.length)
        console.error(
          `  status icons not in GRF: ${[...new Set(statusMissing)].sort().join(", ")}`,
        );
    }

    // Character-creation UI elements, keyed by their original basename
    // (bt_male_on, img_hairstyle_girl05, color03_press, ...).
    for (const [name, entry] of idx) {
      if (!name.startsWith(UI_DIR)) continue;
      const base = name.slice(UI_DIR.length, -".bmp".length);
      if (!/^[a-z0-9_]+$/.test(base)) continue; // flat lowercase names only
      writeIcon("ui", base, entry);
    }

    console.error(
      `\nIcons (PNG) → ${root}\n  item: ${counts.item}  collection: ${counts.collection}  skill: ${counts.skill}  job: ${counts.job}  status: ${counts.status}  ui: ${counts.ui}` +
        (fails.extract ? `\n  ${fails.extract} entry(s) failed to extract` : "") +
        (fails.convert ? `\n  ${fails.convert} BMP(s) skipped (unsupported encoding)` : ""),
    );
    if (jobFailed.length)
      console.error(`  job ids not written: ${jobFailed.sort((a, b) => a - b).join(", ")}`);
  } finally {
    closeGrf(grf);
  }
}

// Pull a small set of named files from an already-open GRF into a name->bytes
// map (keyed by the wanted path), without reopening the archive.
function collectGrfFiles(grf, wants) {
  const map = new Map();
  for (const want of wants) {
    const entry = findBestEntry(grf, want);
    if (entry) {
      try {
        map.set(want, extractFile(grf, entry));
      } catch {
        /* skip */
      }
    }
  }
  return map;
}

// ---------------------------------------------------------------------------
// Card illustration extraction — the full-size (300x400) card artwork.
//
//   <out>/card/<id>.png    from data/texture/유저인터페이스/cardbmp/<name>.bmp
//
// This is NOT what --icons produces for a card: every card shares one generic
// 24x24 inventory icon and one generic 75x100 collection image (their
// iteminfo resource name is literally 이름없는카드, "nameless card"), so the
// per-card picture exists only here. Served at /illust/card/<id>.png rather
// than under /icons — it's an illustration, not an icon.
//
// The id → file link is the client's own data/num2cardillustnametable.txt: a
// EUC-KR text table of "<item id>#<bmp basename>#" lines, "//" comments.
// ---------------------------------------------------------------------------

const CARDBMP_DIR = `${UI}/cardbmp/`;
const CARD_ILLUST_TABLE = "data/num2cardillustnametable.txt";
// The client's "no illustration yet" bitmap. A late block of the table re-points
// ~190 ids at it, in a few cases over a name whose real art is still shipped.
const CARD_PLACEHOLDER = "sorry";

// Parse num2cardillustnametable.txt → Map(id → [names, in file order]). Ids do
// repeat, so every name is kept and the pick is left to pickCardIllust(). Lines
// are decoded one at a time, the way GRF entry names are: the table is EUC-KR
// Korean with ASCII names mixed in, and a single undecodable line must not drag
// the whole table onto the CP1252 fallback.
export function parseCardIllustTable(bytes) {
  const table = new Map();
  for (const raw of Buffer.from(bytes).toString("latin1").split(/\r?\n/)) {
    const line = raw.trim();
    if (!line || line.startsWith("//")) continue;
    const m = /^([0-9]+)#([^#]+)#/.exec(line);
    if (!m) continue;
    const name = decodeName(Buffer.from(m[2], "latin1")).trim();
    if (!name) continue;
    const prev = table.get(m[1]);
    if (prev) prev.push(name);
    else table.set(m[1], [name]);
  }
  return table;
}

// Choose which of an id's names to draw, given has(name) → is that BMP shipped.
// Take the first name that resolves to real art: the table's later "sorry" block
// otherwise buries the 마신의정수 cards' shipped illustrations, and a first name
// that was never shipped (4557 약화된펜릴카드) still falls through to the later
// one that was. Returns null when only the placeholder — or nothing — resolves;
// a 404 is the honest answer for "this card has no art", and the alternative is
// ~190 copies of the same apology bitmap.
export function pickCardIllust(names, has) {
  for (const name of names) {
    if (name.toLowerCase() === CARD_PLACEHOLDER) continue;
    if (has(name)) return name;
  }
  return null;
}

function extractIllust(grfPath, outBase) {
  const grf = openGrf(grfPath);
  try {
    const root = resolve(outBase);
    const cardDir = join(root, "card");
    mkdirSync(cardDir, { recursive: true });

    const tableEntry = findBestEntry(grf, normalize(CARD_ILLUST_TABLE));
    if (!tableEntry) throw new Error(`${CARD_ILLUST_TABLE} not found in GRF`);
    const table = parseCardIllustTable(extractFile(grf, tableEntry));
    console.error(`num2cardillustnametable.txt: ${table.size} card ids`);

    // Index the cardbmp folder (largest copy wins, as everywhere else).
    const idx = new Map();
    for (const f of grf.files) {
      if (!(f.flags & 0x01)) continue;
      const n = normalize(f.filename);
      if (!n.startsWith(CARDBMP_DIR) || !n.endsWith(".bmp")) continue;
      const prev = idx.get(n);
      if (!prev || f.uncompSize > prev.uncompSize) idx.set(n, f);
    }
    console.error(`  ${idx.size} card illustrations indexed`);

    const entryFor = (name) => idx.get(normalize(`${CARDBMP_DIR}${name}.bmp`));
    let written = 0;
    let placeholder = 0;
    const fails = { extract: 0, convert: 0 };
    const missing = [];
    for (const [id, names] of table) {
      const pick = pickCardIllust(names, (n) => entryFor(n) !== undefined);
      if (!pick) {
        if (names.some((n) => entryFor(n))) placeholder++;
        else missing.push(`${id} (${names.join(", ")})`);
        continue;
      }
      let bmp;
      try {
        bmp = extractFile(grf, entryFor(pick));
      } catch {
        fails.extract++;
        continue;
      }
      // Card art is full-bleed and the client draws it opaque, so a magenta
      // pixel here is part of the picture rather than a colorkey.
      const png = bmpToPng(bmp, { opaque: true });
      if (!png) {
        fails.convert++;
        continue;
      }
      writeFileSync(join(cardDir, `${id}.png`), png);
      written++;
    }

    console.error(
      `\nIllustrations (PNG) → ${root}\n  card: ${written}` +
        (placeholder ? `\n  ${placeholder} id(s) skipped — only the "${CARD_PLACEHOLDER}" placeholder` : "") +
        (fails.extract ? `\n  ${fails.extract} entry(s) failed to extract` : "") +
        (fails.convert ? `\n  ${fails.convert} BMP(s) skipped (unsupported encoding)` : ""),
    );
    if (missing.length) console.error(`  not in GRF: ${missing.join(", ")}`);
  } finally {
    closeGrf(grf);
  }
}

// ---------------------------------------------------------------------------
// Effect-only costumes (.str world effects) — auras, falling petals, spotlights,
// ghosts. These costumes carry NO character-sprite view (ClassNum == 0 and the
// resource name isn't in the accessory/robe sprite tables), so the 2D paper-doll
// renderer can't draw them; the client draws them with its ".str" effect system.
// For the latamvisuais 3D map simulator we extract each one's .str into a small
// bundle (effect.json describing the keyframe animation + the texture PNGs it
// references) and a catalogue, served like /icons.
//
// This is the production generalization of latamvisuais/tools/build-effects.mjs
// (the 5-effect POC); the enumeration mirrors that repo's build-db.mjs so the set
// matches exactly the costumes build-db DROPS as effect-only.
// ---------------------------------------------------------------------------

// Read a Lua data table from the GRF (.lub preferred, .lua fallback). Mirrors
// the helper the costume builder uses in latamvisuais/tools/build-db.mjs.
function grfLub(grf, base) {
  const entry = findBestEntry(grf, normalize(`${base}.lub`)) ?? findBestEntry(grf, normalize(`${base}.lua`));
  if (!entry) {
    console.error(`  ! missing in GRF: ${base}.lub`);
    return null;
  }
  return extractFile(grf, entry);
}

// "Equipa em: ^777777Topo e Meio^000000" → ["top","mid"]. Newer LATAM items write
// "Posição: Topo" instead; both labels are accepted. Color codes (^RRGGBB) are
// stripped first. Ported verbatim from build-db.mjs so the slot set matches.
function parseSlots(desc) {
  if (!(desc instanceof LuaTable)) return [];
  for (const line of desc.map.values()) {
    if (typeof line !== "string") continue;
    const s = decodeClientString(line).replace(/\^[0-9a-fA-F]{6}/g, "");
    const m = s.match(/(?:Equipa em|Posi[çc][ãa]o)\s*:\s*(.+)/i);
    if (!m) continue;
    const t = m[1].split(/\s+\S+\s*:/)[0].toLowerCase();
    const slots = [];
    if (t.includes("topo")) slots.push("top");
    if (t.includes("meio")) slots.push("mid");
    if (t.includes("baixo") || /(^|\s)ixo\b/.test(t)) slots.push("low");
    if (t.includes("capa")) slots.push("garment");
    return slots;
  }
  return [];
}

// Normalize a resource name to its effect key / lookup form: forward slashes,
// no leading underscore, lowercase. (Used both as the served effect key and as
// the substring to find the .str folder.)
function normRes(s) {
  return typeof s === "string" ? s.replace(/\\/g, "/").replace(/^_/, "").toLowerCase() : "";
}

// The accessory folder, as the client names it (mirrors the Go resolver's
// kAccessory) — the .act lookup below is the only place this module needs it.
const kACCESSORY = "악세사리";
// The item sprite folder, and the suffix ("_effect") marking the separate
// looping sprite a hat-effect costume plays — see hasHatEffect below.
const kITEM = "아이템";
const kHAT_EFFECT_SUFFIX = "_이펙트";
// The effect sprite folder — where the client keeps the played .spr/.act of every
// `type:"SPR"` effect — and the monster folder a few of them really live in
// (see sprEffectCandidates).
const kEFFECT = "이팩트";
const kMONSTER = "몬스터";

// The sprite a "hat effect" costume plays, from its accessory-table sprite name
// ("_C홍염의폭렬파동" → data/sprite/아이템/c홍염의폭렬파동_이펙트), without the
// extension — the caller looks up the .spr/.act pair. Such a costume ships an
// accessory sprite that is deliberately blank and puts its real visual in this
// separate looping sprite, which the client's hat-effect table calls type "SPR"
// and plays at the character's head; the renderer composites it (gateway
// internal/render, loadHatEffect), so the view does draw after all. Exactly one
// file in the whole GRF carries the suffix today — the one 31089 [Visual] Fúria
// dos Shuras (view 1500) plays — so the name alone identifies it.
export function hatEffectSprite(accName) {
  const base = normRes(accName);
  return base ? `data/sprite/${kITEM}/${base}${kHAT_EFFECT_SUFFIX}` : "";
}

// Robe folders that are nothing but the copied 모험가배낭 template: every .spr in
// them is a backpack leftover (see --prune-robes) and no folder-root .spr exists,
// so the garment has no artwork anywhere in the client. Its real visual is a .str
// world effect, which is what --effects picks up once drawsNothing agrees.
//
// c_snow_powder (view 100) is the only one in this client: all 267 of its per-job
// sprites decode to the backpack, the folder has no root .spr, and the client
// ships data/texture/effect/efst_snow_powder/ssnnnn2.str — which resolveStr finds
// on its own from the de-prefixed resource name, so no STR_OVERRIDE is needed.
// --prune-robes reports any folder it empties, which is how a new one shows up.
const GARMENT_TEMPLATE_ONLY = new Set(["c_snow_powder"]);

// Reverse lookup (sprite name → view id) over the client's accessory/robe name
// tables — the same authority build-db.mjs uses to recover a costume's view when
// ClassNum is 0. A costume that resolves to a view here is a renderable body
// sprite (NOT an effect-only costume), so we use this only to EXCLUDE those.
function buildViewResolver(grf) {
  const tablesFrom = (...bases) => {
    const g = new LuaTable();
    for (const base of bases) {
      const bytes = grfLub(grf, `data/luafiles514/lua files/datainfo/${base}`);
      if (bytes) {
        try {
          runChunkInto(bytes, g);
        } catch (err) {
          console.error(`  ! ${base}: ${err.message}`);
        }
      }
    }
    return g;
  };
  const reverse = (...tables) => {
    const m = new Map();
    for (const t of tables) {
      if (!(t instanceof LuaTable)) continue;
      for (const [k, v] of t.map) {
        const key = normRes(typeof v === "string" ? decodeClientString(v) : "");
        if (typeof k !== "number" || k <= 0 || !key) continue;
        const prev = m.get(key);
        if (prev == null || k < prev) m.set(key, k); // lowest id wins (deterministic)
      }
    }
    return m;
  };
  // view id -> the table's own sprite name, kept alongside the reverse map so a
  // resolved view can be walked back to the .act it renders (see drawsNothing).
  // Unlike normRes this keeps the leading underscore: the sprite file is the
  // gender prefix concatenated with the table value verbatim ("남" + "_c골드샤워").
  const byId = (...tables) => {
    const m = new Map();
    for (const t of tables) {
      if (!(t instanceof LuaTable)) continue;
      for (const [k, v] of t.map) {
        if (typeof k !== "number" || k <= 0 || typeof v !== "string") continue;
        const name = decodeClientString(v).replace(/\\/g, "/").toLowerCase();
        if (name && !m.has(k)) m.set(k, name); // first table wins
      }
    }
    return m;
  };
  const accG = tablesFrom("accessoryid", "accname");
  const robeG = tablesFrom("spriterobeid", "spriterobename");
  const acc = reverse(accG.get("AccNameTable"));
  const robe = reverse(robeG.get("RobeNameTable"), robeG.get("RobeNameTable_Eng"));
  const accById = byId(accG.get("AccNameTable"));
  const robeById = byId(robeG.get("RobeNameTable"), robeG.get("RobeNameTable_Eng"));


  // The view a costume renders with when iteminfo's ClassNum is 0 — common on
  // newer costumes — recovered from the item's sprite resource name.
  const resolveView = (slots, resourceName) => {
    const key = normRes(decodeClientString(resourceName));
    if (!key) return undefined;
    return (slots.includes("garment") ? robe : acc).get(key);
  };

  // Which sprite table the view actually lives in. Normally that follows the
  // equip slot (robe for Capa, accessory for the head slots), but a handful of
  // items' description slot disagrees with where their sprite really is, and
  // rendering them from the wrong table draws the wrong thing. Undefined when
  // both tables (or neither) claim the id, i.e. when there is nothing to say.
  const spriteKind = (view, resourceName) => {
    const key = normRes(decodeClientString(resourceName));
    if (!key || view == null) return undefined;
    const isAcc = acc.get(key) === view;
    const isRobe = robe.get(key) === view;
    if (isAcc === isRobe) return undefined;
    return isAcc ? "headgear" : "garment";
  };

  // Accessory .act entries, indexed on first use: drawsNothing runs for every
  // view in items.json, and a findBestEntry scan per view walks all 269k rows.
  let accActs = null;
  const accAct = (path) => {
    if (accActs === null) {
      accActs = new Map();
      const prefix = normalize(`data/sprite/${kACCESSORY}/`);
      for (const f of grf.files) {
        if (!(f.flags & 0x01)) continue;
        const n = normalize(f.filename);
        if (!n.startsWith(prefix) || !n.endsWith(".act")) continue;
        const prev = accActs.get(n);
        if (!prev || f.uncompSize > prev.uncompSize) accActs.set(n, f); // largest copy wins
      }
    }
    return accActs.get(path);
  };

  // Whether a blank accessory belongs to a "hat effect" costume — one that ships
  // the sprite named by hatEffectSprite. Called only for the handful of blank
  // views, since findBestEntry walks every GRF entry.
  const hasHatEffect = (name) => {
    const base = hatEffectSprite(name);
    return Boolean(base && findBestEntry(grf, normalize(base + ".spr")) && findBestEntry(grf, normalize(base + ".act")));
  };

  // A handful of costumes ship an accessory sprite that is deliberately blank:
  // every .act layer is tinted alpha 0, so the client draws nothing itself and
  // what the player actually sees is a separate effect (the falling
  // petals/feathers ones, 골드샤워, 홍염의폭렬파동, …). They resolve a view like
  // any other costume, so the view alone cannot tell them apart from a real
  // headgear — read the .act. Only accessories do this: not one of the client's
  // 77k robe .act files is fully transparent. A blank .act whose costume has a
  // hat-effect sprite still draws, so it is not reported here — what it renders
  // is that sprite, not the accessory.
  //
  // A garment says the same thing a different way, so it gets its own branch: its
  // .act files are ordinary, but the folder ships no artwork at all — every .spr
  // in it is an adventurer-backpack leftover and there is no folder-root .spr to
  // fall back to. See GARMENT_TEMPLATE_ONLY.
  const blankCache = new Map();
  const drawsNothing = (view, slots) => {
    if (view == null) return false;
    if (slots.includes("garment")) return GARMENT_TEMPLATE_ONLY.has(robeById.get(view) || "");
    if (blankCache.has(view)) return blankCache.get(view);
    let blank = false;
    const name = accById.get(view);
    // Both genders ship the same geometry; the male sprite stands for the pair.
    const entry = name && accAct(normalize(`data/sprite/${kACCESSORY}/남/남${name}.act`));
    if (entry) {
      try {
        blank = actDrawsNothing(extractFile(grf, entry)) && !hasHatEffect(name);
      } catch (err) {
        console.error(`  ! act ${name}: ${err.message}`);
      }
    }
    blankCache.set(view, blank);
    return blank;
  };

  return { resolveView, spriteKind, drawsNothing };
}


// Whether an .act never puts a pixel on screen: every layer of every frame is
// tinted with alpha 0. The renderer (like zrenderer and the client) skips those
// layers, so such a sprite is a placeholder for an effect, not a visual.
export function actDrawsNothing(bytes) {
  let layers = 0;
  for (const action of parseActFrames(bytes).actions) {
    for (const frame of action) {
      for (const layer of frame) {
        if (layer.color[3] !== 0) return false;
        layers++;
      }
    }
  }
  return layers > 0;
}

// Enumerate the effect-only costumes from System/iteminfo_new.lub: costume==true,
// a parsed visual slot, and nothing the character renderer can draw — either no
// resolvable view at all, or a view whose sprite is blank by design and has no
// hat-effect sprite behind it (the set build-db drops). The "invisible" costumes
// (가린다/Invisível — res 인비지블*) hide gear and have no visual to extract, so
// they're excluded up front.
function buildEffectCostumes(grf, args) {
  const lubPath = resolveItemInfoPath(args);
  if (!lubPath) {
    throw new Error("iteminfo_new.lub not found next to the GRF (System/) — pass --iteminfo <path>");
  }
  const tbl = runChunk(readFileSync(lubPath)).get("tbl");
  if (!(tbl instanceof LuaTable)) throw new Error("iteminfo: no `tbl` global");
  const { resolveView, drawsNothing } = buildViewResolver(grf);

  const effects = [];
  const excluded = [];
  for (const [id, entry] of tbl.map) {
    if (typeof id !== "number" || !(entry instanceof LuaTable)) continue;
    if (entry.get("costume") !== true) continue;
    const name = decodeClientString(entry.get("identifiedDisplayName"));
    if (!name) continue;
    const slots = parseSlots(entry.get("identifiedDescriptionName"));
    if (!slots.length) continue;

    // Renderable? (iteminfo carries the view, or its resource name resolves to
    // one) — and does that view actually draw? A view whose .act is entirely
    // alpha-0 renders as nothing, which is how the client says "the visual is an
    // effect, not a sprite"; those stay in this set. A hat effect draws that
    // effect from a sprite rather than a .str, so the renderer handles it and it
    // leaves here (drawsNothing already accounts for it).
    const cn = entry.get("ClassNum");
    const view =
      typeof cn === "number" && cn > 0
        ? Math.round(cn)
        : resolveView(slots, entry.get("identifiedResourceName"));
    if (view != null && !drawsNothing(view, slots)) continue;

    const res = decodeClientString(entry.get("identifiedResourceName")) || "";
    // "Invisible" costumes hide gear — no .str to extract.
    if (/^인비지블/.test(res) || /invis[íi]vel/i.test(name)) {
      excluded.push({ id, name, res });
      continue;
    }
    effects.push({ id, name, slots, res });
  }
  effects.sort((a, b) => a.id - b.id);
  excluded.sort((a, b) => a.id - b.id);
  return { effects, excluded };
}

// Index every .str under data/texture/effect/ (normalized, forward-slash paths).
function indexStrFiles(grf) {
  const out = [];
  for (const f of grf.files) {
    if (!(f.flags & 0x01)) continue;
    const n = normalize(f.filename);
    if (n.startsWith("data/texture/effect/") && n.endsWith(".str")) out.push(n);
  }
  return out;
}

// Manual .str overrides for resource names the heuristic can't pick on its own:
// folders with several real .str where the right one isn't the name-matching one
// (verified visually against the live client). Korean-named and EXE/shared-bound
// effects (the level auras, magic circles, …) whose .str path isn't derivable
// from the resource name go here too once mapped — until then they report as
// unresolved (expected manual follow-up, per the project brief).
const STR_OVERRIDE = {
  // Both of these folders hold two .str files, and the client's hat-effect table
  // (see below) names which one the costume plays — the sibling is dead weight
  // no client table references. vortexf.str even points at a texture the GRF no
  // longer ships, so one of its layers draws nothing; vortexf2.str is clean.
  c_sakura_fubuki: "data/texture/effect/efst_c_sakura_fubuki/sakura_fubuki.str",
  c_swirling_flame: "data/texture/effect/c_swirling_flame/vortexf2.str",
  // Magic circles: the effect folder collapses the resource name's punctuation
  // ("magic_circle" → efst_magiccircle). The rainbow folder holds mc.str AND
  // mcr.str — mcr ("magic circle rainbow") is the rainbow variant.
  magic_circle: "data/texture/effect/efst_magiccircle/mc.str",
  c_magic_circle_rainbow: "data/texture/effect/efst_magiccirclerainbow/mcr.str",
  // Korean-named costumes whose effect folder is romanized (the brief's
  // fluttering/feather/angel_wing hint). Keyed by the normalized resource name;
  // their served key is derived from the .str folder (see effectKey).
  "c흩날리는천사의날개": "data/texture/effect/efst_angel_fluttering/angel_fluttering.str",
  "c흩날리는깃털": "data/texture/effect/efst_feather_fluttering/feath.str",
  "눈의선물": "data/texture/effect/efst_gift_of_snow/gift_of_snow.str",
  // The rest of the romanized set, taken from the client's own hat-effect table
  // (HatEffectInfo/HatEffectInfo.lub maps each HAT_EF_* id to the exact .str it
  // plays) rather than guessed from the folder name. That table is also what
  // settles the ambiguous folders below — it names one .str per effect, so
  // efst_maple_falls resolves to maple_falls.str and not dandan1.str.
  "흩날리는낙엽": "data/texture/effect/efst_maple_falls/maple_falls.str", //          HAT_EF_Maple_Falls
  "흩날리는벚꽃": "data/texture/effect/efst_blossom_fluttering/sakura.str", //        HAT_EF_Blossom_Fluttering
  "c골드샤워": "data/texture/effect/efst_gold_shower/coin2.str", //                   HAT_EF_gold_shower
  "음계의오오라": "data/texture/effect/efst_decoration_of_music/note_1.str", //       HAT_EF_decoration_of_music
  "토끼리본모자": "data/texture/effect/efst_rabbit_aura/toto.str", //                 HAT_EF_rabbit_aura
  teaparty_wonderland: "data/texture/effect/efst_alice_tea/alice02.str", //          HAT_EF_alice_tea
};

// ---------------------------------------------------------------------------
// "Pedras Gráficas" — the visual enchants sold at Malangdo's Loja Fashion.
//
// A stone is not a costume. It is applied INTO a costume already equipped in a
// position, and what the player then sees is a *hat effect*: the same .str world
// effect system the effect-only costumes above use, so the same bundle builder
// serves it. The client models each stone as two items — the tradeable stone
// (whose name is the only place the target position is written) and the enchant
// it becomes once applied, which is the one the server attaches the effect to.
//
// The chain is stone → enchant → HAT_EF_* → .str. The client owns the second
// half of it: HatEffectInfo.lub maps every HAT_EF_* to the .str it plays, and
// readHatEffectTable reads that live, so a client update repaths these on its
// own. It does NOT own the first half — nothing in the client links an item id
// to a HAT_EF_*; the server does, by running the enchant's `hateffect` script.
// So that link is the table below, transcribed from rAthena's item_db (the
// public projection of those scripts), keyed by the STONE's id because that is
// what the consumer lists. Its rows are cross-checked against the enchant's own
// resourceName wherever that name is specific rather than the generic
// 블루크리스탈조각 ("blue crystal fragment"): 벚꽃뱃지 → HAT_EF_CHERRYBLOSSOM,
// C홍염의폭렬파동 → HAT_EF_DOUBLEGUMGANG (both read "crimson explosion wave"),
// ROS_RedSpirit_Effect → HAT_EF_ROS_RedSpirit, 발자국이펙트 → the FOOTPRINT_EF_*.
//
// Roughly half of them name a HAT_EF_* the client compiles into its executable
// (a hatEffectID row with no resourceFileName — Miniatura, Palidez, Sombra, …).
// Those have no asset anywhere and are permanently unrenderable; they stay in
// the table so the report can account for all 29, and drop out on their own
// because readHatEffectTable gives them no .str.
export const STONE_HAT_EFFECT = {
  25058: "HAT_EF_LJOSALFAR", //            Cintilação (Topo)                  enchant 29041
  25059: "HAT_EF_C_Ghost_Effect", //       Fantasmas (Meio)                   enchant 29040
  25136: "HAT_EF_Electric", //             Raios Azuis (Meio)                 enchant 29142
  25137: "HAT_EF_Green_Floor", //          Aura Verde (Baixo)                 enchant 29143
  25138: "HAT_EF_Shrink", //               Miniatura (Meio)                   enchant 29144
  25176: "HAT_EF_CIRCLEPOWER", //          Aura Azul (Meio)                   enchant 29160
  25177: "HAT_EF_KAGEMUSYA", //            Sombra (Meio)                      enchant 29162
  25178: "HAT_EF_CHERRYBLOSSOM", //        Bolha Rosa (Meio)                  enchant 29161
  25205: "HAT_EF_Shrink", //               Miniatura (Baixo)                  enchant 29144
  25224: "HAT_EF_WHITEBODY2", //           Palidez (Meio)                     enchant 29224
  // Raios Vermelhos is the one row where the client outvotes rAthena, which
  // says HAT_EF_DOUBLEGUMGANG (effect 418 — a row the effect table doesn't even
  // have, so nothing would draw). The enchant's own resourceName is
  // C홍염의폭렬파동, which is exactly the sprite HAT_EF_BAKURETSU_HADOU plays
  // (effect 1130, see SPR_EFFECT_OVERRIDE) — 폭렬파동 is 爆裂波動, "bakuretsu
  // hadou" — and that sprite draws red rays, which is what the item is named
  // and described as. Both readings are a hatEffectID either way, so this
  // changes the report, not stones.json.
  25225: "HAT_EF_BAKURETSU_HADOU", //      Raios Vermelhos (Meio)             enchant 29226
  25226: "HAT_EF_WATER_BELOW4", //         Poça d'Água (Baixo)                enchant 29225
  1000319: "HAT_EF_Blessing_Of_Angels", // Luz Angelical (Meio)               enchant 300152
  1000345: "HAT_EF_C_Time_Accessory", //   Relógios (Meio)                    enchant 300157
  1000365: "HAT_EF_magical_feather", //    Corações (Topo)                    enchant 310322
  1000875: "HAT_EF_Valhalla_Idol", //      Popstar (Meio)                     enchant 300385
  1001069: "HAT_EF_Camellia_Hair_Pin", //  Camélia (Topo)                     enchant 300419
  1001175: "HAT_EF_C_Romance_Rose_TW", //  Rosas Românticas (Meio)            enchant 311926
  1001252: "HAT_EF_ResonateTaego", //      Dragão Alado (Topo)                enchant 29727
  1001907: "FOOTPRINT_EF_dumpling", //     Pegada: Bolinhos (Capa)            enchant 313558
  1001908: "FOOTPRINT_EF_panda_basic", //  Pegada: Pandas (Capa)              enchant 313559
  1001909: "FOOTPRINT_EF_panda_color", //  Pegada: Pandas Coloridos (Capa)    enchant 313560
  1002193: "HAT_EF_ROS_BlueSpirit", //     Espírito de Liderança (Meio)       enchant 314094
  1002194: "HAT_EF_ROS_RedSpirit", //      Espírito de Influência (Meio)      enchant 314095
  1002239: "FOOTPRINT_EF_dragon_face_2d", // Pegadas do Banguela (Capa)       enchant 314171
  1002240: "FOOTPRINT_EF_dragon_face_3d", // Pulinhos do Banguela (Capa)      enchant 314172
  // Ventania and Espaço Digital are the only two stones rAthena has no script
  // for. Both are read off the enchant's own aegisName instead, which names a
  // HatEFID constant outright: Golden_Aura_TW → HAT_EF_Golden_Aura_TW (a
  // constant this client declares but then leaves out of hatEffectTable
  // entirely — no .str, no built-in id, nothing to draw at all), and
  // Magic_D_Space_BTM → HAT_EF_Digital_Space (built into the executable).
  1002293: "HAT_EF_Golden_Aura_TW", //     Ventania (Baixo)                   enchant 314198
  1002585: "HAT_EF_Digital_Space", //      Espaço Digital (Baixo)             enchant 314802
  1002642: "FOOTPRINT_EF_phoenix", //      Pegada: Operação Ave de Fogo (Capa) enchant 314813
};

// Read the client's hat-effect table: HAT_EF_* name → what it plays. The two
// chunks have to share globals — HatEffectInfo.lub writes its rows as
// `hatEffectTable[HatEFID.HAT_EF_X]`, so without HatEffectIDs.lub loaded first
// every key evaluates to nil and the whole table collapses onto one row.
//
// A row carries EITHER a `resourceFileName` (the .str it plays, relative to
// data/texture/effect/) OR a `hatEffectID` (an effect compiled into the client
// executable, with no asset in the GRF). Footprints live in a second table,
// FootPrintEffectTable, keyed by the same ids — they are a different animation
// shape (a decal stamped per footstep, four .str for bottom/top × left/right,
// spaced along the walk line by a handful of numbers) rather than one looping
// effect, so they are bundled the same way but catalogued in footprints.json.
function readHatEffectTable(grf) {
  const globals = new LuaTable();
  for (const base of ["HatEffectIDs", "HatEffectInfo", "FootPrintEffectInfo"]) {
    const bytes = grfLub(grf, `data/luafiles514/lua files/HatEffectInfo/${base}`);
    if (!bytes) return null;
    try {
      runChunkInto(bytes, globals);
    } catch (err) {
      console.error(`  ! ${base}.lub: ${err.message}`);
      return null;
    }
  }
  return hatEffectIndex(
    globals.get("HatEFID"),
    globals.get("hatEffectTable"),
    globals.get("FootPrintEffectTable"),
  );
}

// The projection of those three tables: HAT_EF_* name (lowercased — rAthena's
// script constants and the client's own casing disagree freely) → what it plays.
// `str` is the GRF path of its .str, `builtin` the executable-compiled effect id
// it falls back on, `footprint` its FootPrintEffectTable row (see footprintRow)
// when it is a footprint rather than a hat effect. A name declared in HatEFID
// with no row in any of the three gets all three null — the client can't draw it
// either.
export function hatEffectIndex(ids, table, foot) {
  if (!(ids instanceof LuaTable) || !(table instanceof LuaTable)) return null;
  const out = new Map();
  for (const [name, id] of ids.map) {
    const row = table.get(id);
    const res = row instanceof LuaTable ? row.get("resourceFileName") : null;
    out.set(String(name).toLowerCase(), {
      id,
      str: typeof res === "string" && res ? normalize(`data/texture/effect/${res}`) : null,
      builtin: row instanceof LuaTable ? (row.get("hatEffectID") ?? null) : null,
      footprint: foot instanceof LuaTable ? footprintRow(foot.get(id)) : null,
    });
  }
  return out;
}

// What the client falls back on when a FootPrintEffectTable row leaves a field
// out — read off HatEffect_F.lub, whose GetFootprintStr* accessors each end in a
// literal (`luac5.1 -l` on it shows the LOADK). They are worth writing down
// because none of them is the neutral value a reader would assume: a row with no
// Scale_Bottom draws at 0.05 rather than 1, and — the one that actually matters —
// a row with no Stride steps every 50, not every 0, which would stamp every
// print of the walk on one spot. Five of our six stones set neither Stride nor
// Gap, so these ARE their spacing.
export const FOOTPRINT_DEFAULTS = { scale: 0.05, heightTop: 0, stride: 50, gap: 2, adjustAngle: false };

// One FootPrintEffectTable row, normalized: the four .str as GRF paths, and the
// placement numbers with the client's own default substituted wherever the row
// is silent, so a consumer never has to know the defaults above. `_Right` falls
// back to `_Left` — the client reads the pair together and most rows name the
// same file twice. An empty string is the table's way of saying "no such half"
// (several footprints are a puff with no ground mark), and stays null here.
export function footprintRow(row) {
  if (!(row instanceof LuaTable)) return null;
  const str = (key) => {
    const v = row.get(key);
    return typeof v === "string" && v ? normalize(`data/texture/effect/${v}`) : null;
  };
  const num = (key, dflt) => {
    const v = row.get(key);
    return typeof v === "number" ? v : dflt;
  };
  const bottomLeft = str("StrFile_Bottom_Left");
  const topLeft = str("StrFile_Top_Left");
  return {
    type: row.get("Type") ?? null,
    bottomLeft,
    bottomRight: str("StrFile_Bottom_Right") ?? bottomLeft,
    topLeft,
    topRight: str("StrFile_Top_Right") ?? topLeft,
    scaleBottom: num("Scale_Bottom", FOOTPRINT_DEFAULTS.scale),
    scaleTop: num("Scale_Top", FOOTPRINT_DEFAULTS.scale),
    heightTop: num("Height_Top", FOOTPRINT_DEFAULTS.heightTop),
    stride: num("Stride", FOOTPRINT_DEFAULTS.stride),
    gap: num("Gap", FOOTPRINT_DEFAULTS.gap),
    adjustAngle: row.get("IsAdjustAngle") === true,
  };
}

// The STR_OVERRIDE of the SPR side: effect ids whose ported `file` names an asset
// this client does not ship, mapped to the GRF path holding the same effect.
const SPR_EFFECT_OVERRIDE = {
  // 1130 EF_BAKURETSU_HADOU is the hat effect of 31089 [Visual] Fúria dos Shuras
  // — HatEFID.HAT_EF_BAKURETSU_HADOU = 47 and hatEffectTable[47].hatEffectID =
  // 1130, the client's own chain (that entry carries no resourceFileName, which
  // is why it has no .str). roBrowser names the sprite bakuretsu_hadou, another
  // client's romanization; this one ships it under the costume's Korean resource
  // name — 폭렬파동 is 爆裂波動, read "bakuretsu hadou", so the two are one effect.
  // The renderer plays the same file for the costume (see hatEffectSprite).
  1130: "data/sprite/아이템/c홍염의폭렬파동_이펙트",
};

// Collapse "a/b/../c" to "a/c". The GRF is a flat name list, so a path that still
// carries ".." matches nothing — it has to be resolved before the lookup.
function collapseDots(path) {
  const out = [];
  for (const seg of path.split("/")) {
    if (seg === "..") out.pop();
    else if (seg !== "." && seg !== "") out.push(seg);
  }
  return out.join("/");
}

// The GRF sprites (no extension) a `type:"SPR"` effect may play, in preference
// order — the caller takes the first that exists. Normally that is just the
// effect table's own file name under the effect sprite folder. Two things make
// that not enough:
//   • SPR_EFFECT_OVERRIDE redirects the ids whose ported name is another client's.
//   • Six names are written "../npc/<name>", walking out of the effect folder into
//     the client's English-named npc tree — so the ".." has to be collapsed. This
//     client keeps only two of those six sprites under npc/; the other four sit in
//     the Korean monster folder under the same basename (identical assets, the two
//     folders are the same tree under two names), so that is the second candidate.
export function sprEffectCandidates(id, file) {
  if (SPR_EFFECT_OVERRIDE[id]) return [SPR_EFFECT_OVERRIDE[id]];
  const out = [collapseDots(`data/sprite/${kEFFECT}/${file}`)];
  if (file.startsWith("../")) out.push(`data/sprite/${kMONSTER}/${file.slice(file.lastIndexOf("/") + 1)}`);
  return out;
}

// The served effect key: the resource name normalized to a URL/path-safe slug.
// Most resource names are already ASCII; the Korean-named ones aren't, so we fall
// back to the .str folder name (minus the efst_ prefix), e.g.
// efst_angel_fluttering → "angel_fluttering", then the .str basename.
export function effectKey(res, strPath) {
  const k = normRes(res);
  if (/^[a-z0-9_]+$/.test(k)) return k;
  const segs = strPath.split("/");
  const folder = (segs[segs.length - 2] || "").replace(/^efst_/, "");
  if (/^[a-z0-9_]+$/.test(folder)) return folder;
  return strBase(strPath);
}

const strBase = (p) => p.slice(p.lastIndexOf("/") + 1).replace(/\.str$/, "");
// The gateway only serves [a-z0-9_] keys (see handleEffect), so a key it would
// reject is a bundle nobody could fetch — the caller reports it instead.
const effectKeyOk = (key) => /^[a-z0-9_]+$/.test(key);
const isMinStr = (p) => strBase(p).startsWith("min_"); // low-spec "minimized" variant

// Map a resource name → a .str path in the GRF. Returns { str } on success, or
// { ambiguous } / null so the caller can report it. The link is the resource
// name: find the .str folder named efst_<res> or <res> (or a basename <res>.str),
// preferring the basename that matches the resource, then the sole real (non-min)
// .str, else flag the folder as ambiguous for the override table. Costume resource
// names often carry a leading "C_" that the effect folder omits (C_InkPainting_Day
// → efst_inkpainting_day), so the de-prefixed form is tried too.
export function resolveStr(strIndex, res) {
  const r = normRes(res);
  if (!r) return null;
  if (STR_OVERRIDE[r]) {
    const ov = normalize(STR_OVERRIDE[r]);
    const hit = strIndex.find((p) => p.endsWith(ov));
    return hit ? { str: hit } : null;
  }
  const variants = r.startsWith("c_") ? [r, r.slice(2)] : [r];
  for (const v of variants) {
    const folderMatch = strIndex.filter((p) => {
      const segs = p.split("/");
      return segs.includes("efst_" + v) || segs.includes(v);
    });
    const pool = (folderMatch.length ? folderMatch : strIndex.filter((p) => strBase(p) === v)).filter(
      (p) => !isMinStr(p),
    );
    if (!pool.length) continue;
    const byName = pool.find((p) => strBase(p) === v);
    if (byName) return { str: byName };
    if (pool.length === 1) return { str: pool[0] };
    return { ambiguous: pool };
  }
  return null;
}

// Resolve a texture referenced by a .str: its own folder first (bespoke
// textures), then the shared effect texture pool, then the global texture root.
function findEffectTexture(grf, strDir, texName) {
  const n = normRes(texName);
  return (
    findBestEntry(grf, normalize(`${strDir}/${n}`)) ||
    findBestEntry(grf, normalize(`data/texture/effect/${n}`)) ||
    findBestEntry(grf, normalize(`data/texture/${n}`))
  );
}

// Parse a binary ".str" (STRM) world-effect. Little-endian. Ported from
// latamvisuais/tools/str.mjs (itself from roBrowser's Loaders/Str.js). Texture
// names are EUC-KR char[128]; keyframes are 124 bytes. Returns the parsed layers
// plus bytesRead/total so the caller can assert a clean round-trip.
function parseStr(bytes) {
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const u8 = new Uint8Array(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const EUCKR = new TextDecoder("euc-kr");
  let off = 0;
  const u32 = () => { const v = view.getUint32(off, true); off += 4; return v; };
  const i32 = () => { const v = view.getInt32(off, true); off += 4; return v; };
  const f32 = () => { const v = view.getFloat32(off, true); off += 4; return v; };
  const floats = (n) => { const a = new Array(n); for (let i = 0; i < n; i++) a[i] = f32(); return a; };
  const str = (n) => {
    let end = off;
    const lim = off + n;
    while (end < lim && u8[end] !== 0) end++;
    const s = EUCKR.decode(u8.subarray(off, end));
    off += n;
    return s;
  };

  const magic = String.fromCharCode(u8[0], u8[1], u8[2], u8[3]);
  off = 4;
  if (magic !== "STRM") throw new Error(`bad STR magic: ${JSON.stringify(magic)}`);
  const version = u32();
  const fps = u32();
  const maxKey = u32();
  const layerNum = u32();
  off += 16; // reserved

  const layers = [];
  for (let l = 0; l < layerNum; l++) {
    const texNum = u32();
    const textures = [];
    for (let t = 0; t < texNum; t++) textures.push(str(128));
    const animNum = u32();
    const anims = [];
    for (let a = 0; a < animNum; a++) {
      anims.push({
        frame: i32(),
        type: u32(),
        pos: floats(2),
        uv: floats(8),
        xy: floats(8),
        aniframe: f32(),
        anitype: u32(),
        delay: f32(),
        angle: f32(),
        color: floats(4),
        srcalpha: u32(),
        destalpha: u32(),
        mtpreset: u32(),
      });
    }
    layers.push({ textures, anims });
  }
  return { version, fps, maxKey, layers, bytesRead: off, total: u8.length };
}

// Extract one effect into <outDir>: parse the .str, emit each referenced texture
// as tex_N.png (deduped by name, shared textures emitted once), and write
// effect.json with the slimmed keyframes the runtime needs. Mirrors the POC's
// buildEffect so the bundle is byte-identical. Returns counts for the report.
function buildEffect(grf, strPath, key, outDir) {
  const entry = findBestEntry(grf, strPath);
  if (!entry) throw new Error(`.str not found: ${strPath}`);
  const str = parseStr(extractFile(grf, entry));
  const strDir = dirname(strPath); // normalized already

  // One shared PNG per distinct texture name. texFile maps in-.str name →
  // emitted filename (or null when missing/undecodable). The running map size is
  // the next tex index — kept faithful to the POC, including counting nulls.
  const texFile = new Map();
  let texMissing = 0;
  const ensureTexture = (name) => {
    const k = normRes(name);
    if (texFile.has(k)) return texFile.get(k);
    const tex = findEffectTexture(grf, strDir, name);
    if (!tex) {
      texMissing++;
      console.error(`  ! texture missing: ${name}`);
      texFile.set(k, null);
      return null;
    }
    const png = effectTextureToPng(extractFile(grf, tex), k);
    if (!png) {
      console.error(`  ! texture decode failed: ${name}`);
      texFile.set(k, null);
      return null;
    }
    const file = `tex_${texFile.size}.png`;
    writeFileSync(join(outDir, file), png);
    texFile.set(k, file);
    return file;
  };

  const layers = str.layers.map((ly) => ({
    textures: ly.textures.map((t) => ensureTexture(t)),
    anims: ly.anims.map((a) => ({
      frame: a.frame,
      type: a.type,
      pos: a.pos,
      xy: a.xy,
      aniframe: a.aniframe,
      angle: a.angle,
      color: a.color,
      src: a.srcalpha,
      dst: a.destalpha,
    })),
  }));

  const maxKey = effectMaxKey(str.maxKey, layers, key);

  writeFileSync(join(outDir, "effect.json"), JSON.stringify({ key, fps: str.fps, maxKey, layers }));
  return { layers: layers.length, textures: texFile.size, texMissing, bytesRead: str.bytesRead, total: str.total };
}

// The animation length the viewer loops on. One shipped .str
// (efst_rabbit_aura/toto.str) carries garbage in that header field —
// 1,835,102,790 for an effect whose last keyframe is 180 — which would leave the
// effect frozen on its first frame forever. Fall back to the last keyframe when
// the header is implausible; 1 of the client's 258 effects needs it and the rest
// are untouched (the largest legitimate maxKey is in the hundreds).
export function effectMaxKey(headerMaxKey, layers, key = "") {
  if (headerMaxKey >= 0 && headerMaxKey <= 10000) return headerMaxKey;
  const lastKey = layers.reduce((m, ly) => ly.anims.reduce((n, a) => Math.max(n, a.frame), m), 0);
  console.error(`  ! ${key}: maxKey ${headerMaxKey} is out of range — using last keyframe ${lastKey}`);
  return lastKey;
}

function extractEffects(grfPath, outBase, args) {
  const grf = openGrf(grfPath);
  try {
    const root = resolve(outBase);
    mkdirSync(root, { recursive: true });

    console.error("Enumerating effect-only costumes from iteminfo…");
    const { effects, excluded } = buildEffectCostumes(grf, args);
    const strIndex = indexStrFiles(grf);
    console.error(`  ${effects.length} effect-only costumes, ${excluded.length} excluded (invisible), ${strIndex.length} .str files indexed`);

    const resolved = [];
    const unresolved = [];
    for (const eff of effects) {
      const r = resolveStr(strIndex, eff.res);
      if (!r || !r.str) {
        unresolved.push({ ...eff, ambiguous: r?.ambiguous });
        continue;
      }
      const key = effectKey(eff.res, r.str);
      if (!effectKeyOk(key)) {
        // The key is the /effects/{key}/ URL segment; the gateway only serves
        // [a-z0-9_] keys. A non-ASCII .str folder would yield an unservable
        // bundle — flag it for a romanized override instead of writing it.
        unresolved.push({ ...eff, error: `unservable key ${JSON.stringify(key)}` });
        console.error(`  ! ${eff.id} ${JSON.stringify(eff.res)}: non-ASCII key ${JSON.stringify(key)} — add a romanized .str folder to STR_OVERRIDE`);
        continue;
      }
      const outDir = join(root, key);
      rmSync(outDir, { recursive: true, force: true });
      mkdirSync(outDir, { recursive: true });
      try {
        const info = buildEffect(grf, r.str, key, outDir);
        const roundTrip = info.bytesRead === info.total ? "" : ` (! str bytesRead ${info.bytesRead}/${info.total})`;
        console.error(
          `  ✓ ${key} (item ${eff.id}) → ${info.layers} layers, ${info.textures} textures` +
            (info.texMissing ? ` (${info.texMissing} missing)` : "") + roundTrip,
        );
        resolved.push({ ...eff, key, str: r.str });
      } catch (err) {
        rmSync(outDir, { recursive: true, force: true });
        unresolved.push({ ...eff, error: err.message });
        console.error(`  ! ${key} (item ${eff.id}): ${err.message}`);
      }
    }

    // Catalogue: view-less costume entries (id/name/slots + the `effect` key that
    // links to the bundle above). The map simulator's loadDb merges these in.
    const items = resolved
      .map((e) => ({ id: e.id, name: e.name, slots: e.slots, effect: e.key }))
      .sort((a, b) => a.id - b.id);
    writeFileSync(join(root, "index.json"), JSON.stringify({ items }));

    // Every key produced so far. The costume pass owns a key outright: the map
    // and stone passes below reuse a bundle it already built rather than
    // rewriting it, so one .str is only ever extracted once.
    const producedKeys = new Set(resolved.map((e) => e.key));

    // Build one .str into /effects/<key>/ and return { key }, or reuse the bundle
    // when an earlier pass already produced that key. { error } instead when the
    // .str will not parse or its key is one the gateway would refuse to serve
    // (handleEffect only routes [a-z0-9_]), so the caller can report it.
    const bundleStr = (strPath, label) => {
      const key = effectKey("", strPath);
      if (!effectKeyOk(key)) {
        console.error(`  ! ${label}: non-ASCII key ${JSON.stringify(key)}`);
        return { error: `unservable key ${JSON.stringify(key)}` };
      }
      if (producedKeys.has(key)) {
        console.error(`  ✓ ${key} (${label}) → reused`);
        return { key };
      }
      const outDir = join(root, key);
      rmSync(outDir, { recursive: true, force: true });
      mkdirSync(outDir, { recursive: true });
      try {
        const info = buildEffect(grf, strPath, key, outDir);
        const roundTrip = info.bytesRead === info.total ? "" : ` (! str bytesRead ${info.bytesRead}/${info.total})`;
        console.error(
          `  ✓ ${key} (${label}) → ${info.layers} layers, ${info.textures} textures` +
            (info.texMissing ? ` (${info.texMissing} missing)` : "") + roundTrip,
        );
        producedKeys.add(key);
        return { key };
      } catch (err) {
        rmSync(outDir, { recursive: true, force: true });
        console.error(`  ! ${key} (${label}): ${err.message}`);
        return { error: err.message };
      }
    };

    // Graphic stones (see STONE_HAT_EFFECT): stone item id → the .str its hat
    // effect plays, resolved through the client's own HatEffectInfo table, and
    // built into the same /effects/<key>/ bundles. Keyed by the STONE's id —
    // the tradeable item the consumer lists — not the enchant's.
    console.error("\nGraphic stones…");
    const hatEffects = readHatEffectTable(grf);
    const stoneItems = [];
    const stoneBuiltin = [];
    const stoneFootprint = [];
    const stoneUnresolved = [];
    for (const [idText, hatEf] of Object.entries(STONE_HAT_EFFECT)) {
      const id = Number(idText);
      const row = hatEffects?.get(hatEf.toLowerCase());
      if (!row) {
        stoneUnresolved.push({ id, hatEf, why: hatEffects ? "not in HatEFID" : "hat-effect table unreadable" });
        continue;
      }
      if (row.footprint) {
        // Not a stones.json row: it goes to footprints.json below, bundles and all.
        stoneFootprint.push({ id, hatEf, fp: row.footprint });
        continue;
      }
      if (!row.str) {
        // No .str, so nothing to bundle either way — but the two reasons differ.
        // A hatEffectID is the client saying "I draw this myself"; no id at all
        // (HAT_EF_Golden_Aura_TW) is a constant declared and then never given a
        // row, which the client can't draw either. Split them so the report can.
        (row.builtin == null ? stoneUnresolved : stoneBuiltin).push({
          id,
          hatEf,
          ...(row.builtin == null ? { why: "no hatEffectTable row" } : { effect: row.builtin }),
        });
        continue;
      }
      // No resource name to key on — the .str folder is the key, exactly as it
      // is for the Korean-named costumes (see effectKey).
      const built = bundleStr(row.str, `stone ${id}, ${hatEf}`);
      if (built.error) {
        stoneUnresolved.push({ id, hatEf, why: built.error });
        continue;
      }
      stoneItems.push({ id, effect: built.key });
    }
    stoneItems.sort((a, b) => a.id - b.id);
    writeFileSync(join(root, "stones.json"), JSON.stringify({ items: stoneItems }));

    // Footprints: the six garment stones whose HAT_EF_* is a FootPrintEffectTable
    // row. Each is two effects per foot — the mark left on the ground and the
    // puff above it — plus the numbers that stamp them along the walk line, so it
    // has no single `effect` key to put in stones.json and gets its own file.
    //
    // Every footprint stone is listed even when nothing bundled: being in this
    // file is what tells the consumer "this draws while the character walks",
    // which is a different thing to say than stones.json's silence, and a row
    // with no bottomLeft still carries that. The .str themselves are ordinary
    // ones — same bundles, same /effects/<key>/ URLs, deduped against every key
    // built so far (the two pandas share one puff).
    console.error("\nFootprints…");
    const footprintItems = [];
    const footprintPartial = [];
    for (const { id, hatEf, fp } of stoneFootprint) {
      const item = { id };
      const missing = [];
      let named = 0;
      for (const side of ["bottomLeft", "bottomRight", "topLeft", "topRight"]) {
        if (!fp[side]) continue;
        named++;
        const built = bundleStr(fp[side], `footprint ${id}, ${hatEf} ${side}`);
        if (built.key) item[side] = built.key;
        else missing.push(`${side}: ${built.error}`);
      }
      // A row that names no .str at all is the PNG shape (Type 3, a plain decal
      // this pass does not extract) or an empty row — either way nothing drew,
      // which the summary must not count as built.
      if (!named) missing.push(`row names no .str (Type ${fp.type})`);
      Object.assign(item, {
        scaleBottom: fp.scaleBottom,
        scaleTop: fp.scaleTop,
        heightTop: fp.heightTop,
        stride: fp.stride,
        gap: fp.gap,
        adjustAngle: fp.adjustAngle,
      });
      if (missing.length) footprintPartial.push({ id, hatEf, missing });
      footprintItems.push(item);
    }
    footprintItems.sort((a, b) => a.id - b.id);
    writeFileSync(join(root, "footprints.json"), JSON.stringify({ items: footprintItems }));

    // In-world map effects: build a /effects/<key>/ bundle for every servable STR
    // effect in the ported EffectTable, so any map's manifest `effects[].str` keys
    // (e.g. iz_dun03's bubble1..bubble4) resolve. The table is the bounded authority,
    // so this is independent of which maps are extracted. Costume keys win on a
    // collision (already produced above). Each key picks the first id-path that
    // exists in the GRF (handles a basename shared across ids, e.g. safetywall).
    const keyPaths = new Map(); // key -> ordered unique candidate paths
    for (const id of Object.keys(EFFECT_STR_TABLE)) {
      for (const { key, path } of effectStrRefs(Number(id))) {
        if (producedKeys.has(key)) continue;
        if (!keyPaths.has(key)) keyPaths.set(key, []);
        const arr = keyPaths.get(key);
        if (!arr.includes(path)) arr.push(path);
      }
    }
    let mapBuilt = 0;
    const mapMissing = [];
    const mapFailed = [];
    for (const [key, paths] of keyPaths) {
      const path = paths.find((p) => findBestEntry(grf, p));
      if (!path) { mapMissing.push(key); continue; }
      const outDir = join(root, key);
      rmSync(outDir, { recursive: true, force: true });
      mkdirSync(outDir, { recursive: true });
      try {
        const info = buildEffect(grf, path, key, outDir);
        const roundTrip = info.bytesRead === info.total ? "" : ` (! str bytesRead ${info.bytesRead}/${info.total})`;
        console.error(
          `  ✓ ${key} (map effect) → ${info.layers} layers, ${info.textures} textures` +
            (info.texMissing ? ` (${info.texMissing} missing)` : "") + roundTrip,
        );
        producedKeys.add(key);
        mapBuilt++;
      } catch (err) {
        rmSync(outDir, { recursive: true, force: true });
        mapFailed.push(key);
        console.error(`  ! ${key} (map effect): ${err.message}`);
      }
    }

    // Sprite-based map effects (EF_TORCH/EF_SMOKE/EF_BANJJAKII): each id's asset is
    // a played .spr/.act, not a .str, so render one bundle per SPRITE_EFFECT_TABLE
    // key into sprites/<key>/ — the map manifests' effects[].sprite refs resolve
    // here. Deduped by key (only a few bundles), independent of which maps extract.
    const spritesRoot = join(root, "sprites");
    let spritesBuilt = 0;
    const spriteFailed = [];
    const seenSpriteKeys = new Set();
    for (const def of Object.values(SPRITE_EFFECT_TABLE)) {
      if (seenSpriteKeys.has(def.key)) continue;
      seenSpriteKeys.add(def.key);
      const outDir = join(spritesRoot, def.key);
      rmSync(outDir, { recursive: true, force: true });
      mkdirSync(outDir, { recursive: true });
      try {
        const info = buildSpriteEffect(grf, def.sprite, def.key, outDir);
        console.error(`  ✓ sprites/${def.key} (map effect) → ${info.frames} frames`);
        spritesBuilt++;
      } catch (err) {
        rmSync(outDir, { recursive: true, force: true });
        spriteFailed.push(def.key);
        console.error(`  ! sprites/${def.key} (map effect): ${err.message}`);
      }
    }

    // Skill SPR effects: every `type:"SPR"` row in the effect table is a played
    // .spr/.act the client loads from data/sprite/이팩트/<file>. Build one bundle per
    // effect id under sprites/eff_<id>/ so the replay viewer's loadSprEntry resolves
    // any SPR effect by id (no per-name slug map). The table `file` is EUC-KR bytes
    // kept as latin1 (like every client string here) — decode before pathing, and
    // let sprEffectCandidates place the names that aren't a plain child of the
    // effect folder (the hat effect 1130, the six "../npc/…" monster bullets).
    let sprEffBuilt = 0;
    const sprEffFailed = [];
    const effTablePath = new URL("./gateway/internal/effect/data/effect_table.json", import.meta.url);
    if (existsSync(effTablePath)) {
      const effTable = JSON.parse(readFileSync(effTablePath, "utf8"));
      for (const [id, parts] of Object.entries(effTable)) {
        if (!Array.isArray(parts)) continue;
        const sprPart = parts.find((p) => p && p.type === "SPR" && typeof p.file === "string");
        if (!sprPart) continue;
        const key = `eff_${id}`;
        if (seenSpriteKeys.has(key)) continue;
        seenSpriteKeys.add(key);
        const name = decodeClientString(sprPart.file);
        const candidates = sprEffectCandidates(id, name);
        // Keep candidates[0] when nothing exists, so the failure names a real path.
        const sprite = candidates.find((c) => findBestEntry(grf, normalize(c + ".spr"))) || candidates[0];
        const label = sprite === `data/sprite/${kEFFECT}/${name}` ? name : `${name} → ${sprite}`;
        const outDir = join(spritesRoot, key);
        rmSync(outDir, { recursive: true, force: true });
        mkdirSync(outDir, { recursive: true });
        try {
          const info = buildSpriteEffect(grf, sprite, key, outDir);
          console.error(`  ✓ sprites/${key} (skill SPR ${label}) → ${info.frames} frames`);
          sprEffBuilt++;
        } catch (err) {
          rmSync(outDir, { recursive: true, force: true });
          sprEffFailed.push(`${id}:${name}`);
        }
      }
      console.error(`  skill SPR effects: ${sprEffBuilt} built` + (sprEffFailed.length ? `, ${sprEffFailed.length} unresolved: [${sprEffFailed.join(", ")}]` : "") + ` → sprites/eff_<id>/`);
    }

    // Report: resolved / unresolved / excluded (the unresolved set is expected
    // manual follow-up — Korean-named and EXE/shared-bound effects).
    console.error(`\nEffects → ${root}`);
    console.error(`  resolved:   ${resolved.length}`);
    console.error(`  unresolved: ${unresolved.length}`);
    console.error(`  excluded:   ${excluded.length}`);
    console.error(`  map effects: ${mapBuilt} built` + (mapMissing.length ? `, ${mapMissing.length} not in GRF` : "") + (mapFailed.length ? `, ${mapFailed.length} failed` : ""));
    console.error(`  sprite effects: ${spritesBuilt} built` + (spriteFailed.length ? `, ${spriteFailed.length} failed: [${spriteFailed.join(", ")}]` : "") + ` → sprites/`);
    console.error(`  catalogue:  index.json (${items.length} items)`);
    console.error(
      `  stones:     ${stoneItems.length} bundled, ${stoneBuiltin.length} built into the client, ` +
        `${stoneFootprint.length} footprints, ${stoneUnresolved.length} unresolved` +
        ` → stones.json`,
    );
    console.error(
      `  footprints: ${footprintItems.length - footprintPartial.length} built` +
        (footprintPartial.length ? `, ${footprintPartial.length} incomplete` : "") +
        ` → footprints.json`,
    );
    if (stoneBuiltin.length) {
      // Most of these have no asset at all (the client draws them procedurally,
      // or tints the body). A couple name an effect id the table does give a
      // played sprite, which the SPR pass above has already bundled — that is
      // not a .str, so it can't be a stones.json key, but it is not nothing
      // either and the report should not call it unrenderable.
      console.error(`\n  Stones skipped (effect compiled into the client, so there is no .str):`);
      for (const x of stoneBuiltin) {
        const spr = existsSync(join(spritesRoot, `eff_${x.effect}`)) ? `\tsprites/eff_${x.effect}/` : "\tno asset";
        console.error(`    ${x.id}\t${x.hatEf}\teffect ${x.effect}${spr}`);
      }
    }
    if (footprintItems.length) {
      // The placement numbers, per footprint — the one part of this the assets
      // do not carry and a consumer cannot infer. `stride`/`gap` are along and
      // across the walk line, `height` how far the top layer floats above the
      // mark; a value in (parentheses) is the client's default for a field the
      // row leaves out (FOOTPRINT_DEFAULTS), not something the row asked for.
      const shown = (v, key) => (v === FOOTPRINT_DEFAULTS[key] ? `(${v})` : `${v}`);
      console.error(`\n  Footprints (scale bottom/top, height, stride, gap, angle — defaults in parens):`);
      for (const it of footprintItems) {
        const hatEf = stoneFootprint.find((x) => x.id === it.id)?.hatEf ?? "";
        const halves = ["bottomLeft", "bottomRight", "topLeft", "topRight"].filter((k) => it[k]).length;
        console.error(
          `    ${it.id}\t${hatEf}\t${halves}/4 bundled` +
            `\tscale ${shown(it.scaleBottom, "scale")}/${shown(it.scaleTop, "scale")}` +
            `\theight ${shown(it.heightTop, "heightTop")}` +
            `\tstride ${shown(it.stride, "stride")}\tgap ${shown(it.gap, "gap")}` +
            `\t${it.adjustAngle ? "angled" : "flat"}`,
        );
      }
    }
    if (footprintPartial.length) {
      console.error(`\n  Footprints incomplete (listed anyway — the consumer needs to know they exist):`);
      for (const x of footprintPartial) console.error(`    ${x.id}\t${x.hatEf}\t${x.missing.join(", ")}`);
    }
    if (stoneUnresolved.length) {
      console.error(`
  Stones unresolved:`);
      for (const x of stoneUnresolved) console.error(`    ${x.id}	${x.hatEf}	${x.why}`);
    }
    if (unresolved.length) {
      console.error(`\n  Unresolved (need a manual STR_OVERRIDE entry):`);
      for (const u of unresolved) {
        const hint = u.ambiguous ? ` — ambiguous: [${u.ambiguous.join(", ")}]` : u.error ? ` — ${u.error}` : "";
        console.error(`    ${u.id}\t${JSON.stringify(u.res)}\t${u.name}${hint}`);
      }
    }
    if (excluded.length) {
      console.error(`\n  Excluded (invisible costumes — no visual):`);
      for (const x of excluded) console.error(`    ${x.id}\t${JSON.stringify(x.res)}\t${x.name}`);
    }
  } finally {
    closeGrf(grf);
  }
}

// ---------------------------------------------------------------------------
// RSW in-world effects (.str) — the .rsw "type 4" effect objects reference a
// numeric effect id; this is the STR-type subset of roBrowser's
// src/DB/Effects/EffectTable.js (id → { type:"STR", file, rand? }), ported here
// so both the map manifest (`effects`) and the --effects extractor can resolve
// an id to its .str asset(s). Only STR effects are ported — the FUNC / 3D /
// CYLINDER / SPR / weather types roBrowser draws procedurally are out of scope
// (e.g. id 45 = EF_FIREFLY is a FUNC and is absent here, so it is skipped).
//
// `file` may carry a "%d" placeholder expanded over `rand:[a,b]` (the client
// picks one at random per spawn — e.g. 109 EF_BUBBLE "bubble%d" [1,4] →
// bubble1..bubble4), a sub-path ("../npc/x", "RL_C_MAKER/cm"), or an EUC-KR
// Korean name (decoded from the source's \xHH escapes).
// ---------------------------------------------------------------------------

const EFFECT_STR_TABLE = {
  10: [{ file: "maemor" }],
  13: [{ file: "effect/safetywall" }],
  23: [{ file: "stonecurse" }],
  25: [{ file: "firewall%d", rand: [1, 2] }],
  28: [{ file: "freeze" }],
  29: [{ file: "lightning" }, { file: "windhit%d", rand: [1, 3] }],
  30: [{ file: "thunderstorm" }],
  40: [{ file: "cross" }],
  41: [{ file: "angelus" }],
  49: [{ file: "firehit%d", rand: [1, 3] }],
  52: [{ file: "windhit%d", rand: [1, 3] }],
  64: [{ file: "arrowshot" }],
  65: [{ file: "invenom" }],
  66: [{ file: "cure" }],
  67: [{ file: "provoke" }],
  68: [{ file: "mvp" }],
  69: [{ file: "skidtrap" }],
  70: [{ file: "brandish" }],
  75: [{ file: "gloria" }],
  76: [{ file: "magnificat" }],
  77: [{ file: "resurrection" }],
  78: [{ file: "recovery" }],
  83: [{ file: "sanctuary" }],
  84: [{ file: "impositio" }],
  85: [{ file: "lexaeterna" }],
  86: [{ file: "aspersio" }],
  87: [{ file: "lexdivina" }],
  88: [{ file: "suffragium" }],
  89: [{ file: "stormgust" }],
  90: [{ file: "lord" }],
  91: [{ file: "benedictio" }],
  92: [{ file: "meteor%d", rand: [1, 4] }],
  94: [{ file: "quagmire", rand: [1, 4] }],
  95: [{ file: "quagmire" }],
  96: [{ file: "firepillar" }],
  97: [{ file: "firepillarbomb" }],
  101: [{ file: "repairweapon" }],
  102: [{ file: "crashearth" }],
  103: [{ file: "weaponperfection" }],
  104: [{ file: "maximizepower" }],
  106: [{ file: "blastmine" }],
  107: [{ file: "claymore" }],
  108: [{ file: "freezing" }],
  109: [{ file: "bubble%d", rand: [1, 4] }],
  110: [{ file: "gaspush" }],
  111: [{ file: "spring" }],
  112: [{ file: "kyrie" }],
  113: [{ file: "magnus" }],
  124: [{ file: "venomdust", rand: [1, 3] }],
  126: [{ file: "poisonreact_1st" }],
  127: [{ file: "poisonreact" }],
  129: [{ file: "venomsplasher" }],
  130: [{ file: "twohand" }],
  131: [{ file: "autocounter" }],
  133: [{ file: "freeze" }],
  134: [{ file: "freezed" }],
  135: [{ file: "icecrash" }],
  136: [{ file: "slowp" }],
  139: [{ file: "sandman" }],
  141: [{ file: "pneuma%d", rand: [1, 3] }],
  143: [{ file: "sonicblow" }],
  144: [{ file: "brandish2" }],
  146: [{ file: "shockwavehit" }],
  147: [{ file: "earthhit" }],
  148: [{ file: "pierce" }],
  149: [{ file: "bowling" }],
  150: [{ file: "spearstab" }],
  151: [{ file: "spearboomerang" }],
  152: [{ file: "holyhit" }],
  153: [{ file: "concentration" }],
  154: [{ file: "bs_refinesuccess" }],
  155: [{ file: "bs_refinefailed" }],
  158: [{ file: "joblvup" }],
  169: [{ file: "energycoat" }],
  170: [{ file: "cartrevolution" }],
  181: [{ file: "mentalbreak" }],
  182: [{ file: "magical" }],
  183: [{ file: "sui_explosion" }],
  185: [{ file: "suicide" }],
  186: [{ file: "yunta_1" }],
  187: [{ file: "yunta_2" }],
  188: [{ file: "yunta_3" }],
  189: [{ file: "yunta_4" }],
  190: [{ file: "yunta_5" }],
  191: [{ file: "homing" }],
  192: [{ file: "poison" }],
  193: [{ file: "silence" }],
  194: [{ file: "stun" }],
  195: [{ file: "stonecurse" }],
  197: [{ file: "sleep" }],
  199: [{ file: "pong%d", rand: [1, 3] }],
  204: [{ file: "빨간포션" }],
  205: [{ file: "주홍포션" }],
  206: [{ file: "노란포션" }],
  207: [{ file: "하얀포션" }],
  208: [{ file: "파란포션" }],
  209: [{ file: "초록포션" }],
  210: [{ file: "fruit" }],
  211: [{ file: "fruit_" }],
  213: [{ file: "deffender" }],
  214: [{ file: "keeping" }],
  218: [{ file: "집중" }],
  219: [{ file: "각성" }],
  220: [{ file: "버서크" }],
  234: [{ file: "spell" }],
  235: [{ file: "디스펠" }],
  244: [{ file: "매직로드" }],
  245: [{ file: "holy_cross" }],
  246: [{ file: "shield_charge" }],
  248: [{ file: "providence" }],
  250: [{ file: "twohand" }],
  251: [{ file: "devotion" }],
  255: [{ file: "enc_fire" }],
  256: [{ file: "enc_ice" }],
  257: [{ file: "enc_wind" }],
  258: [{ file: "enc_earth" }],
  268: [{ file: "steal_coin" }],
  269: [{ file: "strip_weapon" }],
  270: [{ file: "strip_shield" }],
  271: [{ file: "strip_armor" }],
  272: [{ file: "strip_helm" }],
  273: [{ file: "연환" }],
  293: [{ file: "유저인터페이스/item/염산병.bmp", rand: [1, 3] }],
  305: [{ file: "p_success" }],
  306: [{ file: "p_failed" }],
  311: [{ file: "loud" }],
  315: [{ file: "safetywall" }],
  337: [{ file: "joblvup" }],
  369: [{ file: "twohand" }],
  371: [{ file: "angel" }],
  372: [{ file: "devil" }],
  390: [{ file: "melt" }],
  391: [{ file: "cart" }],
  392: [{ file: "sword" }],
  406: [{ file: "소울번" }],
  407: [{ file: "사람효과" }],
  440: [{ file: "asum" }],
  452: [{ file: "스톱", rand: [1, 3] }],
  491: [{ file: "찹쌀떡" }],
  492: [{ file: "ramadan" }],
  507: [{ file: "mapae" }],
  508: [{ file: "itempokjuk" }],
  565: [{ file: "moonlight_1" }],
  566: [{ file: "moonlight_2" }],
  567: [{ file: "moonlight_3" }],
  568: [{ file: "h_levelup" }],
  569: [{ file: "defense" }],
  593: [{ file: "food_str" }],
  594: [{ file: "food_int" }],
  595: [{ file: "food_vit" }],
  596: [{ file: "food_agi" }],
  597: [{ file: "food_dex" }],
  598: [{ file: "food_luk" }],
  603: [{ file: "firehit%d", rand: [1, 3] }],
  608: [{ file: "cook_suc" }],
  609: [{ file: "cook_fail" }],
  612: [{ file: "itempokjuk" }],
  618: [{ file: "firehit", rand: [1, 3] }],
  619: [{ file: "freeze", rand: [1, 3] }],
  622: [{ file: "setsudan" }],
  635: [{ file: "fire dragon" }],
  636: [{ file: "icy" }],
  646: [{ file: "트랙킹" }],
  649: [{ file: "불스아이" }],
  668: [{ file: "dragon_h" }],
  669: [{ file: "wideb" }],
  670: [{ file: "dfear" }],
  677: [{ file: "cwound" }],
  682: [{ file: "itempokjuk" }],
  683: [{ file: "itempokjuk" }],
  684: [{ file: "itempokjuk" }],
  685: [{ file: "itempokjuk" }],
  686: [{ file: "itempokjuk" }],
  699: [{ file: "flower_leaf" }],
  704: [{ file: "mobile_ef02" }],
  705: [{ file: "mobile_ef01" }],
  706: [{ file: "mobile_ef03" }],
  708: [{ file: "storm_min" }],
  709: [{ file: "pokjuk_jap" }],
  717: [{ file: "angelus" }],
  721: [{ file: "ado" }],
  722: [{ file: "이그니션브레이크" }],
  727: [{ file: "crimson_r" }],
  728: [{ file: "hell_in" }],
  731: [{ file: "dragon_h" }],
  734: [{ file: "chainlight" }],
  745: [{ file: "aimed" }],
  746: [{ file: "arrowstorm" }],
  747: [{ file: "laulamus" }],
  748: [{ file: "lauagnus" }],
  749: [{ file: "mil_shield" }],
  750: [{ file: "concentration" }],
  756: [{ file: "버서크" }],
  795: [{ file: "powerswing" }],
  813: [{ file: "enervation" }],
  814: [{ file: "groomy" }],
  815: [{ file: "ignorance" }],
  816: [{ file: "laziness" }],
  817: [{ file: "unlucky" }],
  818: [{ file: "weakness" }],
  920: [{ file: "firewall_per" }],
  926: [{ file: "hunter_shockwave_blue" }],
  959: [{ file: "poison_mist" }],
  960: [{ file: "eraser_cutter" }],
  964: [{ file: "lava_slide" }],
  965: [{ file: "sonic_claw" }],
  966: [{ file: "tinder" }],
  967: [{ file: "mid_frenzy" }],
  975: [{ file: "vash00" }],
  987: [{ file: "rwc2011" }],
  988: [{ file: "rwc2011_2" }],
  1015: [{ file: "rune_success" }],
  1016: [{ file: "rune_fail" }],
  1017: [{ file: "changematerial_su" }],
  1018: [{ file: "changematerial_fa" }],
  1019: [{ file: "guardian" }],
  1020: [{ file: "bubble%d_1", rand: [1, 4] }],
  1021: [{ file: "dust" }],
  1029: [{ file: "dancingblade" }],
  1031: [{ file: "invincibleoff2" }],
  1033: [{ file: "devil" }],
  1040: [{ file: "gc_darkcrow" }],
  1042: [{ file: "all_full_throttle" }],
  1043: [{ file: "sr_flashcombo" }],
  1044: [{ file: "rk_luxanima" }],
  1046: [{ file: "so_elemental_shield" }],
  1047: [{ file: "ab_offertorium" }],
  1048: [{ file: "wl_telekinesis_intense" }],
  1049: [{ file: "gn_illusiondoping" }],
  1050: [{ file: "nc_magma_eruption" }],
  1055: [{ file: "chill" }],
  1057: [{ file: "ab_offertorium_ring" }],
  1062: [{ file: "stormgust" }],
  1094: [{ file: "ach_complete/ppring3" }],
  1186: [{ file: "new_dropitem/dropitem_pink/dropitem_pink/dropitem_pink" }, { file: "new_dropitem/dropitem_pink/dropitem_pink_bottom/dropitem_pink_bottom" }],
  1189: [{ file: "new_dropitem/dropitem_yellow/dropitem_yellow/dropitem_yellow" }, { file: "new_dropitem/dropitem_yellow/dropitem_yellow_bottom/dropitem_yellow_bottom" }],
  1190: [{ file: "new_dropitem/dropitem_purple/dropitem_purple/dropitem_purple" }, { file: "new_dropitem/dropitem_purple/dropitem_purple_bottom/dropitem_purple_bottom" }],
  1869: [{ file: "new_dropitem/dropitem_blue/dropitem_blue/dropitem_blue" }, { file: "new_dropitem/dropitem_blue/dropitem_blue_bottom/dropitem_blue_bottom" }],
  1870: [{ file: "new_dropitem/dropitem_green/dropitem_green/dropitem_green" }, { file: "new_dropitem/dropitem_green/dropitem_green_bottom/dropitem_green_bottom" }],
  1871: [{ file: "new_dropitem/dropitem_red/dropitem_red/dropitem_red" }, { file: "new_dropitem/dropitem_red/dropitem_red_bottom/dropitem_red_bottom" }],
  1872: [{ file: "grade_enchant/new_success/new_success" }],
  1873: [{ file: "grade_enchant/new_failed/new_failed" }],
  1874: [{ file: "grade_enchant/new_intro/new_intro" }],
  1875: [{ file: "ui_enchant/ui_intro_yellow/ui_intro_yellow" }],
  1876: [{ file: "ui_enchant/ui_enchant_success/ui_enchant_success" }],
  1877: [{ file: "ui_enchant/ui_fail/ui_enchant_fail" }],
  1878: [{ file: "ui_enchant/ui_intro_blue/ui_intro_blue" }],
  1879: [{ file: "ui_enchant/ui_levelup_success/ui_levelup_success" }],
  1880: [{ file: "ui_enchant/ui_fail/ui_levelup_fail" }],
  1881: [{ file: "ui_enchant/ui_intro_green/ui_intro_green" }],
  1882: [{ file: "ui_enchant/ui_reset_success/ui_reset_success" }],
  1883: [{ file: "ui_enchant/ui_fail/ui_reset_fail" }],};

// Expand a STR def's `file` to concrete .str references. A "%d" placeholder over
// rand:[a,b] yields one name per integer in the (inclusive) range; otherwise the
// file is taken verbatim (a bare `rand` without "%d" is a render hint we ignore).
export function expandStrFiles(def) {
  if (def.rand && def.file.includes("%d")) {
    const out = [];
    for (let i = def.rand[0]; i <= def.rand[1]; i++) out.push(def.file.replace(/%d/g, String(i)));
    return out;
  }
  return [def.file];
}

// The served effect key: the .str basename as a URL-safe [a-z0-9_] segment (the
// /effects/<key>/ path the gateway serves). Returns null for non-ASCII names
// (the Korean-named effects) — those can't be served, so they're skipped in v1.
export function effectStrKey(file) {
  const base = normRes(file).split("/").pop();
  return /^[a-z0-9_]+$/.test(base) ? base : null;
}

// Resolve a .str `file` token to its normalized GRF path under data/texture/effect/,
// folding any leading "../" (e.g. "../npc/x" → data/texture/npc/x.str).
export function effectStrPath(file) {
  const parts = [];
  for (const seg of normalize(`data/texture/effect/${file}.str`).split("/")) {
    if (seg === "..") parts.pop();
    else if (seg) parts.push(seg);
  }
  return parts.join("/");
}

// All servable .str references for an effect id: { key, path } per expanded name,
// deduped by key (order preserved). Returns [] for ids that aren't STR effects
// (FUNC/3D/weather/unknown) or whose names are all unservable. Shared by the map
// manifest (`str` = the keys) and the --effects extractor (builds each path once).
export function effectStrRefs(id) {
  const defs = EFFECT_STR_TABLE[id];
  if (!defs) return [];
  const seen = new Set();
  const refs = [];
  for (const def of defs) {
    for (const file of expandStrFiles(def)) {
      const key = effectStrKey(file);
      if (!key || seen.has(key)) continue;
      seen.add(key);
      refs.push({ key, path: effectStrPath(file) });
    }
  }
  return refs;
}

// ---------------------------------------------------------------------------
// RSW in-world effects (sprite) — a second family of .rsw type-4 ids whose asset
// is a *played sprite* (.spr/.act), not a .str: roBrowser's EffectTable types
// '3D' (a billboarded sprite played frame-by-frame) and 'SPR'. Ported from
// src/DB/Effects/EffectTable.js. Each entry's `sprite` is the GRF sprite path
// (the 이팩트 = "effect" sprite folder, an EUC-KR name), and `key` is the URL-safe
// slug the bundle is served under (/effects/sprites/<key>/) and the value the map
// manifest carries in effects[].sprite. The --effects step renders every entry's
// frames once (see buildSpriteEffect) so any map's sprite refs resolve.
const SPRITE_EFFECT_TABLE = {
  44: { key: "smoke", sprite: "data/sprite/이팩트/굴뚝연기" }, //   EF_SMOKE — chimney smoke (3D)
  47: { key: "torch_01", sprite: "data/sprite/이팩트/torch_01" }, // EF_TORCH — looping flame (3D)
  165: { key: "banjjakii", sprite: "data/sprite/이팩트/크리스마스" }, // EF_BANJJAKII — Comodo fireworks ball (SPR)
};

// .rsw type-4 ids the client renders procedurally from no shippable asset, but
// that still need a placement baked (just id + pos) so the client can spawn them
// in-world. 45 EF_FIREFLY (type FUNC) — faint drifting motes the client generates
// itself. Distinct from the EXE-bound hardcoded ids (torch_red/pillar/…), which
// have neither a data asset nor a roBrowser implementation, so stay unbaked.
const FUNC_EFFECT_IDS = new Set([45]);

// ---------------------------------------------------------------------------
// Map extraction — world maps (.gat/.gnd/.rsw + referenced .rsm models and
// BMP/TGA textures) for the latamvisuais 3D map simulator (src/sim).
//
// Each map's geometry binaries are emitted raw (parsed in the browser); the
// models, textures, water frames and shared cursor/grid UI are de-duplicated
// across all 900+ maps into content-addressed stores so identical blobs are
// stored (and served) exactly once:
//
//   <out>/<map>/<map>.gat|gnd|rsw   raw geometry (browser-parsed)
//   <out>/<map>/manifest.json       resolves resource names → shared blob paths
//   <out>/_m/<hash>.rsm             a referenced model (raw)
//   <out>/_t/<hash>.png             a referenced texture (BMP/TGA → transparent PNG)
//   <out>/_w/<hash>.jpg             one animated-water frame (raw JPG)
//   <out>/_u/<hash>.png             a shared UI image (grid selector / cursor frame)
//   <out>/index.json                { maps: [...] } — every extracted map name
//
// Manifest blob paths are written relative to the map dir as "../_t/<hash>.png"
// etc.; the browser fetches them as `baseUrl + path`, and the URL parser folds
// the ".." so they resolve against the store, not the map dir. This mirrors the
// proof-of-concept latamvisuais/tools/build-map.mjs (same manifest shape) but
// shares blobs instead of copying them per map.
// ---------------------------------------------------------------------------

const MAP_EUCKR = new TextDecoder("euc-kr");

// Little-endian binary cursor over a Uint8Array (ported from roformat.mjs).
class MapReader {
  constructor(bytes) {
    this.b = bytes;
    this.dv = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
    this.p = 0;
  }
  // Fixed-length, NUL-terminated EUC-KR string field.
  str(n) {
    let end = this.p;
    const lim = this.p + n;
    while (end < lim && this.b[end] !== 0) end++;
    const s = MAP_EUCKR.decode(this.b.subarray(this.p, end));
    this.p += n;
    return s;
  }
  lstr() { return this.str(this.u32()); } // u32 length then that many bytes (RSW/RSM 2.x)
  u8() { return this.b[this.p++]; }
  i8() { const v = this.dv.getInt8(this.p); this.p += 1; return v; }
  u32() { const v = this.dv.getUint32(this.p, true); this.p += 4; return v; }
  i32() { const v = this.dv.getInt32(this.p, true); this.p += 4; return v; }
  f32() { const v = this.dv.getFloat32(this.p, true); this.p += 4; return v; }
  seek(n) { this.p += n; }
}

// Normalize an embedded resource name (EUC-KR, backslash-separated) to the
// lowercase forward-slash key both the manifest and the browser parsers use.
function normName(name) {
  return name.replace(/\\/g, "/").replace(/^\/+/, "").toLowerCase();
}

// RSW → referenced model filenames + water type. Field layout ported from
// roBrowserLegacy (handles RSW 1.x–2.x). We read only the object list.
export function parseRsw(bytes) {
  const fp = new MapReader(bytes);
  if (fp.str(4) !== "GRSW") throw new Error("RSW: bad header");
  const version = fp.i8() + fp.i8() / 10;

  if (version >= 2.5) fp.i32(); // build number
  if (version >= 2.2) fp.u8(); // unknown byte

  fp.str(40); // ini
  fp.str(40); // gnd
  fp.str(40); // gat
  if (version >= 1.4) fp.str(40); // src

  let waterType = 0;
  if (version < 2.6) {
    if (version >= 1.3) fp.f32(); // water level
    if (version >= 1.8) { waterType = fp.i32(); fp.f32(); fp.f32(); fp.f32(); } // type, waveH, waveSpeed, wavePitch
    if (version >= 1.9) fp.i32(); // animSpeed
  }
  if (version >= 1.5) {
    fp.i32(); fp.i32(); // longitude, latitude
    fp.f32(); fp.f32(); fp.f32(); // diffuse
    fp.f32(); fp.f32(); fp.f32(); // ambient
    if (version >= 1.7) fp.f32(); // opacity
  }
  if (version >= 1.6) { fp.i32(); fp.i32(); fp.i32(); fp.i32(); } // ground bounds
  if (version >= 2.7) { const c = fp.i32(); fp.seek(4 * c); }

  const count = fp.i32();
  const models = [];
  const effects = [];
  for (let i = 0; i < count; i++) {
    const type = fp.i32();
    if (type === 1) {
      if (version >= 1.3) { fp.str(40); fp.i32(); fp.f32(); fp.i32(); } // name, animType, animSpeed, blockType
      if (version >= 2.6) fp.u8();
      if (version >= 2.7) fp.i32();
      const filename = fp.str(80);
      fp.str(80); // node name
      fp.f32(); fp.f32(); fp.f32(); // position
      fp.f32(); fp.f32(); fp.f32(); // rotation
      fp.f32(); fp.f32(); fp.f32(); // scale
      models.push(filename);
    } else if (type === 2) {
      fp.str(80); fp.f32(); fp.f32(); fp.f32(); fp.i32(); fp.i32(); fp.i32(); fp.f32();
    } else if (type === 3) {
      fp.str(80); fp.str(80); fp.f32(); fp.f32(); fp.f32(); fp.f32(); fp.i32(); fp.i32(); fp.f32();
      if (version >= 2.0) fp.f32();
    } else if (type === 4) {
      // In-world effect: name(80), pos[3]÷5, id(long), delay(float), param[4]. The
      // id maps to a .str world effect via EFFECT_STR_TABLE (EF_BUBBLE = 109 etc.).
      // Positions match the roBrowser ÷5 world scale (same as model/light/sound).
      fp.str(80); // name (unused)
      const pos = [fp.f32() / 5, fp.f32() / 5, fp.f32() / 5];
      const id = fp.i32();
      const delay = fp.f32(); // raw .rsw delay (roBrowser scales it ×10 at render time)
      const param = [fp.f32(), fp.f32(), fp.f32(), fp.f32()];
      effects.push({ id, pos, delay, param });
    } else {
      break; // unknown — stop (quadtree/footer follows the object list anyway)
    }
  }
  return { models: [...new Set(models)], waterType, effects };
}

// GND → ground texture filenames (relative to data/texture/).
function parseGndTextures(bytes) {
  const fp = new MapReader(bytes);
  if (fp.str(4) !== "GRGN") throw new Error("GND: bad header");
  fp.i8(); fp.i8(); // version
  fp.u32(); fp.u32(); // width, height
  fp.f32(); // zoom
  const count = fp.u32();
  const length = fp.u32();
  const textures = [];
  for (let i = 0; i < count; i++) textures.push(fp.str(length));
  return [...new Set(textures)];
}

// RSM → texture filenames (relative to data/texture/). For <2.2 every name is in
// the top-level list (40-char strings); 2.2/2.3 length-prefix them and (2.3)
// carry them per node, so we walk the node tree collecting string entries.
function parseRsmTextures(bytes) {
  const fp = new MapReader(bytes);
  const header = fp.str(4);
  if (header !== "GRSM" && header !== "GRSX") throw new Error("RSM: bad header");
  const version = fp.i8() + fp.i8() / 10;
  fp.i32(); // animLen
  fp.i32(); // shadeType
  if (version >= 1.4) fp.u8(); // alpha

  const textures = [];
  if (version >= 2.3) {
    fp.f32(); // frame rate
    const c = fp.u32();
    for (let i = 0; i < c; i++) textures.push(fp.lstr());
  } else if (version >= 2.2) {
    fp.f32();
    const ac = fp.u32();
    for (let i = 0; i < ac; i++) textures.push(fp.lstr());
    const c = fp.u32();
    for (let i = 0; i < c; i++) textures.push(fp.lstr());
  } else {
    fp.seek(16); // reserved
    const c = fp.u32();
    for (let i = 0; i < c; i++) textures.push(fp.str(40));
    fp.str(40); // main node name (not a texture)
    return [...new Set(textures)]; // <2.2: node textures are indices, nothing new
  }

  // 2.2/2.3: descend nodes to gather any per-node string textures.
  const nodeCount = fp.u32();
  for (let n = 0; n < nodeCount; n++) {
    fp.lstr(); // name
    fp.lstr(); // parent name
    const tc = fp.u32();
    for (let i = 0; i < tc; i++) {
      if (version >= 2.3) textures.push(fp.lstr());
      else fp.i32(); // texture index
    }
    fp.seek(9 * 4 + 3 * 4); // mat3 + offset
    if (version < 2.2) fp.seek(10 * 4); // pos/rotangle/rotaxis/scale (absent for >=2.2)
    const vc = fp.u32(); fp.seek(vc * 12);
    const tvc = fp.u32(); fp.seek(tvc * (version >= 1.2 ? 12 : 8));
    const fc = fp.u32();
    for (let i = 0; i < fc; i++) {
      if (version >= 2.2) fp.seek(fp.i32()); // length-prefixed face record
      else fp.seek(version >= 1.2 ? 24 : 20);
    }
    if (version >= 1.6) { const sc = fp.u32(); fp.seek(sc * 20); } // scale keyframes
    const rc = fp.u32(); fp.seek(rc * 20); // rot keyframes
    if (version >= 2.2) { const pc = fp.u32(); fp.seek(pc * 20); } // pos keyframes
    if (version >= 2.3) {
      const g = fp.u32();
      for (let i = 0; i < g; i++) {
        fp.i32(); // texture id
        const anims = fp.u32();
        for (let a = 0; a < anims; a++) {
          fp.i32(); // type
          const frames = fp.u32();
          fp.seek(frames * 8); // frame i32 + offset f32
        }
      }
    }
  }
  return [...new Set(textures)];
}

// Decode SPR frame `index` (default 0) → { width, height, rgba }; palette index
// 0 = transparent. Just enough to pull the cursor frames (ported from spr.mjs).
function decodeSprFrame(bytes, index = 0) {
  const dv = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  let p = 0;
  const u8 = () => bytes[p++];
  const u16 = () => { const v = dv.getUint16(p, true); p += 2; return v; };

  if (bytes[0] !== 0x53 || bytes[1] !== 0x50) throw new Error("SPR: bad header"); // "SP"
  p = 2;
  const minor = u8();
  const major = u8();
  const version = major + minor / 10;

  const indexedCount = u16();
  if (version > 1.1) u16(); // rgba frame count (unused — palette frames only)
  if (index >= indexedCount) throw new Error("SPR: frame index out of range (palette frames only)");

  const palStart = bytes.length - 1024; // palette is the last 1024 bytes

  let frame = null;
  for (let i = 0; i <= index; i++) {
    const width = u16();
    const height = u16();
    const size = width * height;
    const data = new Uint8Array(size);
    if (version < 2.1) {
      for (let k = 0; k < size; k++) data[k] = bytes[p++];
    } else {
      const end = u16() + p; // RLE: a run of zeros is 0x00 then a count
      let idx = 0;
      while (p < end) {
        const c = bytes[p++];
        data[idx++] = c;
        if (!c) {
          const count = bytes[p++];
          if (!count) data[idx++] = 0;
          else for (let j = 1; j < count; j++) data[idx++] = c;
        }
      }
    }
    if (i === index) frame = { width, height, data };
  }

  const { width, height, data } = frame;
  const rgba = new Uint8Array(width * height * 4);
  for (let i = 0; i < width * height; i++) {
    const pi = data[i] * 4;
    rgba[i * 4] = bytes[palStart + pi];
    rgba[i * 4 + 1] = bytes[palStart + pi + 1];
    rgba[i * 4 + 2] = bytes[palStart + pi + 2];
    rgba[i * 4 + 3] = data[i] === 0 ? 0 : 255; // index 0 = transparent
  }
  return { width, height, rgba };
}

// ACT → per-action playback sequence: out[action] is the layer-0 SPR frame index
// of each of that action's animations, in order (ported from act.mjs).
function actActionSequences(bytes) {
  const dv = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  let p = 0;
  const u8 = () => bytes[p++];
  const u16 = () => { const v = dv.getUint16(p, true); p += 2; return v; };
  const u32 = () => { const v = dv.getUint32(p, true); p += 4; return v; };
  const i32 = () => { const v = dv.getInt32(p, true); p += 4; return v; };
  const f32 = () => { const v = dv.getFloat32(p, true); p += 4; return v; };
  const seek = (n) => { p += n; };

  if (bytes[0] !== 0x41 || bytes[1] !== 0x43) throw new Error("ACT: bad header");
  p = 2;
  const minor = u8();
  const major = u8();
  const version = major + minor / 10;

  const actionCount = u16();
  seek(10);
  const out = [];

  for (let a = 0; a < actionCount; a++) {
    const animCount = u32();
    const seq = [];
    for (let an = 0; an < animCount; an++) {
      seek(32);
      const layerCount = u32();
      let first = -1;
      for (let l = 0; l < layerCount; l++) {
        seek(8);
        const index = i32();
        i32();
        if (version >= 2.0) {
          seek(4); f32();
          if (version > 2.3) f32();
          i32(); i32();
          if (version >= 2.5) { i32(); i32(); }
        }
        if (l === 0) first = index;
      }
      if (version >= 2.0) i32();
      if (version >= 2.3) { const c = i32(); for (let i = 0; i < c; i++) seek(12); }
      seq.push(first);
    }
    out.push(seq);
  }
  return out;
}

// Decode every frame of a .spr into { width, height, rgba, type } in roBrowser
// frame order — the palette-indexed frames (type 0, palette index 0 =
// transparent) first, then the RGBA (truecolor) frames (type 1). RGBA frames are
// stored ABGR and are swizzled to RGBA here. `type` matches a .act layer's sprite
// type and lets a layer's index resolve into the right list (the two families are
// indexed independently, exactly like the renderer's spr.images[sprType][index]).
// Used to render the sprite-based map effects (EF_TORCH/EF_SMOKE/EF_BANJJAKII);
// decodeSprFrame above pulls a single palette frame (the cursor) and can't read
// these.
export function decodeSprFrames(bytes) {
  const dv = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  let p = 0;
  const u8 = () => bytes[p++];
  const u16 = () => { const v = dv.getUint16(p, true); p += 2; return v; };
  if (bytes[0] !== 0x53 || bytes[1] !== 0x50) throw new Error("SPR: bad header"); // "SP"
  p = 2;
  const minor = u8();
  const major = u8();
  const version = major + minor / 10;
  const palCount = u16();
  const rgbaCount = version > 1.1 ? u16() : 0;

  const palStart = bytes.length - 1024; // palette is the trailing 1024 bytes
  const frames = [];

  // Palette-indexed frames (same decode as decodeSprFrame, every frame).
  for (let f = 0; f < palCount; f++) {
    const width = u16();
    const height = u16();
    const size = width * height;
    const data = new Uint8Array(size);
    if (version < 2.1) {
      for (let k = 0; k < size; k++) data[k] = bytes[p++];
    } else {
      const end = u16() + p; // RLE: a run of zeros is 0x00 then a count
      let idx = 0;
      while (p < end) {
        const c = bytes[p++];
        data[idx++] = c;
        if (!c) {
          const count = bytes[p++];
          if (!count) data[idx++] = 0;
          else for (let j = 1; j < count; j++) data[idx++] = c;
        }
      }
    }
    const rgba = new Uint8Array(size * 4);
    for (let i = 0; i < size; i++) {
      const pi = data[i] * 4;
      rgba[i * 4] = bytes[palStart + pi];
      rgba[i * 4 + 1] = bytes[palStart + pi + 1];
      rgba[i * 4 + 2] = bytes[palStart + pi + 2];
      rgba[i * 4 + 3] = data[i] === 0 ? 0 : 255;
    }
    frames.push({ width, height, rgba, type: 0 });
  }

  // RGBA (truecolor) frames — raw width*height*4 bytes, stored ABGR.
  for (let f = 0; f < rgbaCount; f++) {
    const width = u16();
    const height = u16();
    const size = width * height;
    const rgba = new Uint8Array(size * 4);
    for (let i = 0; i < size; i++) {
      const a = bytes[p], b = bytes[p + 1], g = bytes[p + 2], r = bytes[p + 3];
      p += 4;
      rgba[i * 4] = r; rgba[i * 4 + 1] = g; rgba[i * 4 + 2] = b; rgba[i * 4 + 3] = a;
    }
    frames.push({ width, height, rgba, type: 1 });
  }
  return frames;
}

// Parse a .act's full playback: per action, a list of frames (motions), each a
// list of sprite layers carrying the placement the renderer needs (x/y offset of
// the layer's centre from the act origin, sprite index + type, mirror, packed
// colour, scaleX/Y, rotation), plus each action's frame delay in ms (the stored
// float ×25). Byte-accurate for the 2.x acts the effect sprites use: the
// version>=2.0 layer carries a 4-byte packed colour (not 4 floats) then
// scaleX/(scaleY)/rotation/spriteType, and the version>=2.3 attach points are 16
// bytes each. (actActionSequences above only balances pre-2.0 layouts — fine for
// the cursor, wrong here.) After the actions come the sound-event table (>=2.1)
// and then the per-action delays (>=2.2). The packed colour is little-endian
// 0xAABBGGRR, so the four bytes in file order are R,G,B,A.
export function parseActFrames(bytes) {
  const dv = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  let p = 0;
  const u8 = () => bytes[p++];
  const u16 = () => { const v = dv.getUint16(p, true); p += 2; return v; };
  const i32 = () => { const v = dv.getInt32(p, true); p += 4; return v; };
  const f32 = () => { const v = dv.getFloat32(p, true); p += 4; return v; };
  const seek = (n) => { p += n; };

  if (bytes[0] !== 0x41 || bytes[1] !== 0x43) throw new Error("ACT: bad header"); // "AC"
  p = 2;
  const minor = u8();
  const major = u8();
  const version = major + minor / 10;

  const actionCount = u16();
  seek(10);
  const actions = [];
  for (let a = 0; a < actionCount; a++) {
    const motionCount = i32();
    const frames = [];
    for (let m = 0; m < motionCount; m++) {
      seek(32); // range1[4] + range2[4]
      const layerCount = i32();
      const layers = [];
      for (let l = 0; l < layerCount; l++) {
        const x = i32();
        const y = i32();
        const index = i32();
        const mirror = i32();
        let color = [255, 255, 255, 255];
        let scaleX = 1, scaleY = 1, rotation = 0, sprType = 0;
        if (version >= 2.0) {
          color = [u8(), u8(), u8(), u8()]; // packed RGBA colour (R,G,B,A bytes)
          scaleX = f32();
          scaleY = version > 2.3 ? f32() : scaleX; // separate scaleY from 2.4
          rotation = i32(); // degrees
          sprType = i32(); // 0 = palette-indexed image, 1 = rgba image
          if (version >= 2.5) { i32(); i32(); } // width, height
        }
        layers.push({ x, y, index, sprType, mirror, color, scaleX, scaleY, rotation });
      }
      if (version >= 2.0) i32(); // sound event id
      if (version >= 2.3) { const c = i32(); seek(c * 16); } // attach points
      frames.push(layers);
    }
    actions.push(frames);
  }
  if (version >= 2.1) { const events = i32(); seek(events * 40); } // sound names
  const delays = [];
  if (version >= 2.2) for (let a = 0; a < actionCount; a++) delays.push(f32() * 25);
  return { actions, delays };
}

// --- ACT frame compositing (sprite-based map effects) -------------------------
// A faithful JS port of the native renderer's per-layer affine placement
// (sprite.transformOfSprite + geom.Transform) and rasteriser (raster.DrawSprite /
// AlphaBlend / TintPixel), just enough to bake one .act frame's sprite stack into
// a single RGBA image. Matrices are row-major length-9; coordinates are RO sprite
// pixels with +x right / +y down and the act origin at (0,0).

const ACT_PI180 = Math.PI / 180;

// Round half away from zero (D's std.math.round, which the renderer matches at
// sub-pixel boundaries — JS Math.round rounds half toward +∞ instead).
const roundHalfAway = (v) => (v < 0 ? -Math.round(-v) : Math.round(v));

function mat3Mul(a, b) {
  const m = new Array(9);
  for (let r = 0; r < 3; r++)
    for (let c = 0; c < 3; c++)
      m[r * 3 + c] = a[r * 3] * b[c] + a[r * 3 + 1] * b[3 + c] + a[r * 3 + 2] * b[6 + c];
  return m;
}

// m · (x, y, 1) → [x', y'].
function mat3Apply(m, x, y) {
  return [m[0] * x + m[1] * y + m[2], m[3] * x + m[4] * y + m[5]];
}

// Matrix inverse, mirroring geom.Mat3.Inverse (det clamped to float epsilon).
function mat3Inverse(m) {
  let det =
    m[0] * m[4] * m[8] + m[1] * m[5] * m[6] + m[2] * m[3] * m[7] -
    m[6] * m[4] * m[2] - m[7] * m[5] * m[0] - m[8] * m[3] * m[1];
  if (det === 0) det = 1.19209290e-7;
  const inv = 1 / det;
  return [
    inv * (m[4] * m[8] - m[5] * m[7]),
    inv * (m[2] * m[7] - m[1] * m[8]),
    inv * (m[1] * m[5] - m[2] * m[4]),
    inv * (m[5] * m[6] - m[3] * m[8]),
    inv * (m[0] * m[8] - m[2] * m[6]),
    inv * (m[2] * m[3] - m[0] * m[5]),
    inv * (m[3] * m[7] - m[4] * m[6]),
    inv * (m[1] * m[6] - m[0] * m[7]),
    inv * (m[0] * m[4] - m[1] * m[3]),
  ];
}

// Build one layer's affine transform and its integer bounding box, mirroring
// sprite.transformOfSprite + geom.Transform.Calculate/BoundingBox. Origin is the
// sprite centre (0.5, 0.5); the composition order is T·R·S·translate(-size/2).
// Returns null when the layer is fully transparent (the renderer skips it).
function actLayerTransform(layer, width, height) {
  if (layer.color[3] === 0) return null;
  let mirror = 1, mirrorAdjust = 0;
  if (layer.mirror & 1) { mirror = -1; mirrorAdjust = 0.5; } // mirror rounding hack
  const sizeX = width - mirrorAdjust, sizeY = height;

  let m = [1, 0, 0, 0, 1, 0, 0, 0, 1];
  m = mat3Mul(m, [1, 0, layer.x, 0, 1, layer.y, 0, 0, 1]); // translation
  const s = Math.sin(layer.rotation * ACT_PI180), c = Math.cos(layer.rotation * ACT_PI180);
  m = mat3Mul(m, [c, -s, 0, s, c, 0, 0, 0, 1]); // rotation
  m = mat3Mul(m, [layer.scaleX * mirror, 0, 0, 0, layer.scaleY, 0, 0, 0, 1]); // scale
  const ox = roundHalfAway(-sizeX * 0.5), oy = roundHalfAway(-sizeY * 0.5);
  m = mat3Mul(m, [1, 0, ox, 0, 1, oy, 0, 0, 1]); // -size*origin

  const corners = [mat3Apply(m, 0, 0), mat3Apply(m, sizeX, 0), mat3Apply(m, 0, sizeY), mat3Apply(m, sizeX, sizeY)];
  let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
  for (const [px, py] of corners) {
    if (px < minX) minX = px; if (py < minY) minY = py;
    if (px > maxX) maxX = px; if (py > maxY) maxY = py;
  }
  // Box bounds truncate toward zero (geom.Box's int() cast).
  return { m, box: { x1: Math.trunc(minX), y1: Math.trunc(minY), x2: Math.trunc(maxX), y2: Math.trunc(maxY) } };
}

// Composite one .act frame's layers (resolved against framesByType, the decoded
// .spr split into [type0, type1] lists) into a single RGBA image at its natural
// bounding size, baking in every layer's placement, scale, rotation, mirror and
// colour. Returns { width, height, rgba, offset:[x,y] } where offset is the centre
// of the composited image relative to the act origin (RO px, +x right / +y down),
// or null when no layer contributes. Ports raster.DrawSprite + AlphaBlend +
// TintPixel exactly (nearest-neighbour inverse sampling, integer alpha math).
export function compositeActFrame(framesByType, layers) {
  const built = [];
  let ux1 = Infinity, uy1 = Infinity, ux2 = -Infinity, uy2 = -Infinity;
  for (const layer of layers) {
    const src = framesByType[layer.sprType] && framesByType[layer.sprType][layer.index];
    if (!src) continue;
    const t = actLayerTransform(layer, src.width, src.height);
    if (!t) continue;
    built.push({ src, m: t.m, box: t.box, color: layer.color });
    if (t.box.x1 < ux1) ux1 = t.box.x1; if (t.box.y1 < uy1) uy1 = t.box.y1;
    if (t.box.x2 > ux2) ux2 = t.box.x2; if (t.box.y2 > uy2) uy2 = t.box.y2;
  }
  if (!built.length) return null;
  const W = Math.abs(ux2 - ux1), H = Math.abs(uy2 - uy1);
  if (W === 0 || H === 0) return null;

  const dest = new Uint8Array(W * H * 4);
  for (const b of built) {
    const w = Math.abs(b.box.x2 - b.box.x1), h = Math.abs(b.box.y2 - b.box.y1);
    if (w === 0 || h === 0) continue;
    const inv = mat3Inverse(b.m);
    const offX = b.box.x1 - ux1, offY = b.box.y1 - uy1;
    const [tr, tg, tb, ta] = b.color;
    const { width: sw, height: sh, rgba: src } = b.src;
    for (let i = 0; i < w * h; i++) {
      const tx = i % w, ty = (i / w) | 0;
      const dx = tx + offX, dy = ty + offY;
      if (dx < 0 || dx >= W || dy < 0 || dy >= H) continue;
      const [sxf, syf] = mat3Apply(inv, tx + b.box.x1, ty + b.box.y1);
      const sx = roundHalfAway(sxf), sy = roundHalfAway(syf);
      if (sx < 0 || sy < 0 || sx >= sw || sy >= sh) continue;
      const si = (sx + sy * sw) * 4;
      if (src[si + 3] === 0) continue;
      // TintPixel: per-channel multiply by the layer colour.
      const cr = (tr * src[si]) / 255 | 0;
      const cg = (tg * src[si + 1]) / 255 | 0;
      const cb = (tb * src[si + 2]) / 255 | 0;
      const ca = (ta * src[si + 3]) / 255 | 0;
      // AlphaBlend src over dest (integer un-premultiply, matching the renderer).
      const di = (dx + dy * W) * 4;
      const da = dest[di + 3];
      if (da === 0 || ca === 255) {
        dest[di] = cr; dest[di + 1] = cg; dest[di + 2] = cb; dest[di + 3] = ca;
        continue;
      }
      const newA = ca + Math.trunc((da * (255 - ca)) / 255);
      if (newA === 0) { dest[di] = dest[di + 1] = dest[di + 2] = dest[di + 3] = 0; continue; }
      const ch = (sc, dc) =>
        Math.trunc((Math.trunc((sc * ca) / 255) + Math.trunc((dc * da * (255 - ca)) / (255 * 255))) * 255 / newA);
      dest[di] = ch(cr, dest[di]);
      dest[di + 1] = ch(cg, dest[di + 1]);
      dest[di + 2] = ch(cb, dest[di + 2]);
      dest[di + 3] = newA;
    }
  }
  return { width: W, height: H, rgba: dest, offset: [(ux1 + ux2) / 2, (uy1 + uy2) / 2] };
}

// Render a sprite-based map effect into a served bundle: <outDir>/<i>.png — one
// composited PNG per frame of the effect's first action (these map effects have a
// single action) — plus sprite.json { frames: [{ img, delay, offset:[x,y] }] }.
// Each PNG bakes in every .act layer's placement, scale, rotation, mirror and
// colour (so it already looks like the in-game frame), and `offset` is the centre
// of that composited image relative to the effect's placement origin (RO px,
// +x right / +y down; the client negates y for its Y-up world). `delay` is the
// action's real frame interval in ms (the .act float ×25), defaulting to 100 when
// it carries no/zero delay.
function buildSpriteEffect(grf, spritePath, key, outDir) {
  const sprEntry = findBestEntry(grf, normalize(spritePath + ".spr"));
  if (!sprEntry) throw new Error("no .spr in GRF");
  const decoded = decodeSprFrames(extractFile(grf, sprEntry));
  if (!decoded.length) throw new Error("no frames");
  // Split into the two index spaces a .act layer's sprType selects between.
  const framesByType = [[], []];
  for (const f of decoded) framesByType[f.type].push(f);

  const actEntry = findBestEntry(grf, normalize(spritePath + ".act"));
  if (!actEntry) throw new Error("no .act in GRF");
  const { actions, delays } = parseActFrames(extractFile(grf, actEntry));
  const action = actions[0];
  if (!action || !action.length) throw new Error("act action 0 has no frames");
  const delay = delays[0] > 0 ? Math.round(delays[0]) : 100;

  const frames = [];
  action.forEach((layers, i) => {
    const frame = compositeActFrame(framesByType, layers);
    if (!frame) return; // a frame with no visible layers contributes nothing
    const img = `${i}.png`;
    writeFileSync(join(outDir, img), encodePng(frame.width, frame.height, Buffer.from(frame.rgba)));
    frames.push({ img, delay, offset: [roundHalfAway(frame.offset[0]), roundHalfAway(frame.offset[1])] });
  });
  if (!frames.length) throw new Error("no composited frames");
  writeFileSync(join(outDir, "sprite.json"), JSON.stringify({ frames }));
  return { frames: frames.length };
}

// Content-addressed blob store: writes each distinct byte payload once under
// <outBase>/<subdir>/<hash>.<ext> and returns its store-relative path. Identical
// blobs (the same texture/model/water frame referenced by many maps) collapse to
// one file. The returned path is later prefixed with "../" in the manifest.
function makeBlobStore(outBase) {
  const seen = new Set();
  return function put(subdir, ext, bytes) {
    const hash = createHash("sha256").update(bytes).digest("hex").slice(0, 16);
    const rel = `${subdir}/${hash}.${ext}`;
    if (!seen.has(rel)) {
      const dir = join(outBase, subdir);
      mkdirSync(dir, { recursive: true });
      writeFileSync(join(dir, `${hash}.${ext}`), bytes);
      seen.add(rel);
    }
    return rel;
  };
}

// findBestEntry scans all ~260k GRF entries per call, which is far too slow for
// the ~100 lookups each of 900+ maps needs. Build the resolution once: a
// normalized-name → best-entry map (largest uncompressed copy wins, matching
// findBestEntry's tie-break) so per-map lookups are O(1). Map resources are
// always full `data/...` paths, so exact-name keying reproduces findBestEntry's
// result for them (its endsWith leniency only matters for partial paths we never
// pass here).
function buildEntryIndex(grf) {
  const idx = new Map();
  for (const f of grf.files) {
    if (!(f.flags & 0x01)) continue;
    const name = normalize(f.filename);
    const prev = idx.get(name);
    if (!prev || f.uncompSize > prev.uncompSize) idx.set(name, f);
  }
  return idx;
}

// Every map name in the GRF (the basename of each data/<name>.rsw), lowercased.
function listMapNames(grf) {
  const names = new Set();
  for (const f of grf.files) {
    if (!(f.flags & 0x01)) continue;
    const m = /^data\/([a-z0-9_@-]+)\.rsw$/.exec(normalize(f.filename));
    if (m) names.add(m[1]);
  }
  return [...names].sort();
}

// The mouse cursor + hovered-cell selector are identical for every map, so we
// extract them once into the shared _u store and reuse the manifest fragment.
function extractMapUi(grf, index, store) {
  const ui = {};
  const gridEntry = index.get("data/texture/grid.tga");
  if (gridEntry) {
    const png = effectTextureToPng(extractFile(grf, gridEntry), "grid.tga");
    if (png) ui.grid = "../" + store("_u", "png", png);
  }

  const cursorSpr = index.get("data/sprite/cursors.spr");
  const cursorAct = index.get("data/sprite/cursors.act");
  if (cursorSpr && cursorAct) {
    const spr = extractFile(grf, cursorSpr);
    const DEFAULT_ACTION = 0; // animated default arrow
    const ROTATE_ACTION = 4; // two-curvy-arrows rotate cursor
    const CURSOR_FPS = 12;
    let seqs = [];
    try {
      seqs = actActionSequences(extractFile(grf, cursorAct));
    } catch (err) {
      console.warn(`  ! cursors.act parse failed: ${err.message}`);
    }
    // Emit the distinct frames of an action's sequence + a seq[] indexing them.
    const buildCursor = (actionSeq) => {
      const order = [];
      const seen = new Map();
      for (const idx of actionSeq) {
        if (idx < 0) continue;
        if (!seen.has(idx)) { seen.set(idx, order.length); order.push(idx); }
      }
      if (!order.length) return null;
      const frames = [];
      let w = 0, h = 0;
      order.forEach((idx, i) => {
        const fr = decodeSprFrame(spr, idx);
        if (i === 0) { w = fr.width; h = fr.height; }
        frames.push("../" + store("_u", "png", encodePng(fr.width, fr.height, Buffer.from(fr.rgba))));
      });
      const seq = actionSeq.filter((i) => i >= 0).map((i) => seen.get(i));
      return { frames, seq, w, h };
    };
    try {
      const def = buildCursor(seqs[DEFAULT_ACTION] ?? [0]);
      if (def) ui.cursor = { frames: def.frames, seq: def.seq, hotspot: [1, 1], fps: CURSOR_FPS, fallback: "default" }; // arrow tip ≈ top-left
      const rot = buildCursor(seqs[ROTATE_ACTION] ?? [10]);
      if (rot) ui.cursorRotate = { frames: rot.frames, seq: rot.seq, hotspot: [Math.round(rot.w / 2), Math.round(rot.h / 2)], fps: CURSOR_FPS, fallback: "grabbing" }; // pivots about centre
    } catch (err) {
      console.warn(`  ! cursor extraction failed: ${err.message}`);
    }
  }
  return ui;
}

// ---------------------------------------------------------------------------
// EffectTool emitter lubs — the modern parametric map effects EF_EMITTER (974),
// EF_ANIMATED_EMITTER (1073) and EF_MAGIC_FLOOR (1025) are NOT .str files. Their
// particle spec lives per-map in `data/luafiles514/lua files/effecttool/<map>.lub`
// as global tables (`_<map>_emitterInfo`/`_animatedEmitterInfo`/`_magicfloorInfo`
// and a generic `_<map>_Effect` container with Type/ID fields). Each .rsw type-4
// placement of these ids matches a lub entry by horizontal (X/Z) position, so we
// attach the matched entry's spec to the manifest effect — the client renders it
// from that, no client EXE needed.
//
// The `*Info` data is ALWAYS a straight-line table constructor; the FORLOOP/CALL
// in these lubs live only in trailing loader closures that must NOT be executed
// (running their control flow corrupts the data). So executeLub runs straight
// line — branch/loop/call/upvalue ops are no-ops, nested closures are captured
// but never run — while still evaluating arithmetic (some coords are computed).
// EffectTool arrays are 0-indexed (keys 0..N); one lub (1@def03) ships as plain
// Lua source rather than LuaQ bytecode, so readEffectToolLub handles both.
// ---------------------------------------------------------------------------

const EMITTER_EFFECT_IDS = new Set([974, 1073, 1025]);

// Straight-line execution of a lub's root proto (see note above). Reuses the
// shared OP/BITRK/FIELDS_PER_FLUSH/LuaTable defined for the iteminfo reader.
function executeLub(proto, globals) {
  const R = [];
  const K = proto.k;
  const rk = (x) => (x & BITRK ? K[x & (BITRK - 1)] : R[x]);
  let pc = 0;
  const code = proto.code;
  while (pc < code.length) {
    const i = code[pc++];
    const op = i & 0x3f;
    const a = (i >>> 6) & 0xff;
    const c = (i >>> 14) & 0x1ff;
    const b = (i >>> 23) & 0x1ff;
    const bx = (i >>> 14) & 0x3ffff;
    switch (op) {
      case OP.MOVE: R[a] = R[b]; break;
      case OP.LOADK: R[a] = K[bx]; break;
      case OP.LOADBOOL: R[a] = b !== 0; if (c) pc++; break;
      case OP.LOADNIL: for (let r = a; r <= b; r++) R[r] = undefined; break;
      case OP.GETGLOBAL: R[a] = globals.get(K[bx]); break;
      case OP.SETGLOBAL: globals.set(K[bx], R[a]); break;
      case OP.NEWTABLE: R[a] = new LuaTable(); break;
      case OP.GETTABLE: { const t = R[b]; R[a] = t instanceof LuaTable ? t.get(rk(c)) : undefined; break; }
      case OP.SETTABLE: { const t = R[a]; if (t instanceof LuaTable) t.set(rk(b), rk(c)); break; }
      case OP.ADD: R[a] = rk(b) + rk(c); break;
      case OP.SUB: R[a] = rk(b) - rk(c); break;
      case OP.MUL: R[a] = rk(b) * rk(c); break;
      case OP.DIV: R[a] = rk(b) / rk(c); break;
      case OP.MOD: { const x = rk(b), y = rk(c); R[a] = x - Math.floor(x / y) * y; break; }
      case OP.POW: R[a] = Math.pow(rk(b), rk(c)); break;
      case OP.UNM: R[a] = -R[b]; break;
      case OP.SETLIST: {
        let n = b;
        let block = c;
        if (block === 0) block = code[pc++]; // real C stored in the next word
        if (n === 0) { n = 0; while (R[a + n + 1] !== undefined) n++; } // B=0: flush to top
        const base = (block - 1) * FIELDS_PER_FLUSH;
        const t = R[a];
        for (let j = 1; j <= n; j++) t.set(base + j, R[a + j]);
        break;
      }
      case OP.CLOSURE: R[a] = { __closure: proto.protos[bx] }; break; // captured, not run
      case OP.RETURN: return;
      default: break; // JMP/FOR*/CALL/EQ/…/upvalues — no-op (straight-line)
    }
  }
}

// LuaTable → plain JS. EffectTool list tables are 0-indexed (keys 0..N) while Lua
// literals like {x,y,z} are 1-indexed, so an all-integer-keyed table is walked
// min..max (walking 1..max would silently drop entry 0).
function lubToJS(v, seen = new Set()) {
  if (v instanceof LuaTable) {
    if (seen.has(v)) return undefined;
    seen.add(v);
    const keys = [...v.map.keys()];
    const arrish = keys.length > 0 && keys.every((k) => typeof k === "number" && Number.isInteger(k));
    if (arrish) {
      const min = Math.min(...keys), max = Math.max(...keys);
      const arr = [];
      for (let i = min; i <= max; i++) arr.push(lubToJS(v.map.get(i), seen));
      return arr;
    }
    const o = {};
    for (const k of keys) o[k] = lubToJS(v.map.get(k), seen);
    return o;
  }
  if (typeof v === "number") return Math.abs(v) < 1e-9 ? 0 : Math.round(v * 1e4) / 1e4;
  return v;
}

// Minimal Lua table-literal parser for the rare uncompiled (plain-text) lub.
function parseLubSource(src) {
  src = src.replace(/--\[\[[\s\S]*?\]\]/g, "").replace(/--[^\n]*/g, "");
  let i = 0;
  const n = src.length;
  const ws = () => { while (i < n && /\s/.test(src[i])) i++; };
  function value() {
    ws();
    const ch = src[i];
    if (ch === "{") return table();
    if (ch === "[" && src[i + 1] === "[") { i += 2; const e = src.indexOf("]]", i); const s = src.slice(i, e); i = e + 2; return s; }
    if (ch === '"' || ch === "'") { const q = ch; i++; let s = ""; while (i < n && src[i] !== q) { if (src[i] === "\\") { s += src[i + 1]; i += 2; } else s += src[i++]; } i++; return s; }
    let j = i; while (j < n && /[^,}\s\]=]/.test(src[j])) j++; const tok = src.slice(i, j); i = j;
    if (tok === "true") return true; if (tok === "false") return false; if (tok === "nil") return undefined;
    const num = Number(tok); return Number.isNaN(num) ? tok : num;
  }
  function table() {
    const t = {}; const arr = []; i++;
    for (;;) {
      ws(); if (src[i] === "}") { i++; break; }
      if (src[i] === "[") {
        i++; ws(); let key;
        if (src[i] === '"' || src[i] === "'") { const q = src[i++]; key = ""; while (src[i] !== q) key += src[i++]; i++; }
        else { let j = i; while (src[j] !== "]") j++; key = Number(src.slice(i, j).trim()); i = j; }
        ws(); i++; ws(); i++; t[key] = value();
      } else if (/[A-Za-z_]/.test(src[i])) {
        let j = i; while (/[A-Za-z0-9_]/.test(src[j])) j++; const key = src.slice(i, j); i = j; ws();
        if (src[i] === "=") { i++; t[key] = value(); } else { i = j; arr.push(value()); }
      } else arr.push(value());
      ws(); if (src[i] === "," || src[i] === ";") i++;
    }
    if (arr.length) arr.forEach((v, k) => (t[k + 1] = v));
    return Object.keys(t).every((k) => /^\d+$/.test(k)) ? Object.values(t) : t;
  }
  const globals = {};
  const re = /(_[A-Za-z0-9_]+)\s*=\s*/g;
  let m;
  while ((m = re.exec(src))) { i = re.lastIndex; ws(); globals[m[1]] = value(); re.lastIndex = i; }
  return globals;
}

// Read an EffectTool lub (LuaQ bytecode or plain-text source) → globals object.
function readEffectToolLub(bytes) {
  if (bytes[0] === 0x1b && bytes[1] === 0x4c && bytes[2] === 0x75 && bytes[3] === 0x61) {
    const g = new LuaTable();
    executeLub(loadChunk(bytes), g);
    const out = {};
    for (const k of g.map.keys()) out[k] = lubToJS(g.get(k));
    return out;
  }
  return parseLubSource(Buffer.from(bytes).toString("latin1"));
}

// All emitter placement records from a map's lub: every global that's a non-empty
// array of objects carrying a `pos` (covers emitterInfo/animatedEmitterInfo/
// magicfloorInfo and the generic Effect container). [] when the map has no lub.
function readMapEmitters(grf, index, map) {
  const entry = index.get(`data/luafiles514/lua files/effecttool/${map}.lub`);
  if (!entry) return [];
  let globals;
  try { globals = readEffectToolLub(extractFile(grf, entry)); }
  catch (err) { console.warn(`  ! ${map}: effecttool lub parse failed: ${err.message}`); return []; }
  const out = [];
  for (const val of Object.values(globals)) {
    if (!Array.isArray(val) || !val.length) continue;
    if (!val.every((e) => e && typeof e === "object" && !Array.isArray(e) && Array.isArray(e.pos))) continue;
    out.push(...val);
  }
  return out;
}

// Extract one map into <outBase>/<map>/ (raw geometry + manifest) plus its
// model/texture/water blobs into the shared store. Returns a stats object, or
// { skipped } when a required geometry file is missing.
function extractOneMap(grf, index, map, outBase, store, ui, fogTable) {
  const rawFiles = {};
  for (const ext of ["gat", "gnd", "rsw"]) {
    const entry = index.get(`data/${map}.${ext}`);
    if (!entry) return { skipped: `no .${ext}` };
    rawFiles[ext] = extractFile(grf, entry);
  }

  const mapDir = join(outBase, map);
  mkdirSync(mapDir, { recursive: true });
  for (const ext of ["gat", "gnd", "rsw"]) writeFileSync(join(mapDir, `${map}.${ext}`), rawFiles[ext]);

  const { models: modelNames, waterType, effects: rswEffects } = parseRsw(rawFiles.rsw);
  const modelMap = {}; // normName -> ../_m/<hash>.rsm
  const textureNames = new Set();
  for (const name of parseGndTextures(rawFiles.gnd)) textureNames.add(normName(name));

  let modelMissing = 0;
  for (const name of modelNames) {
    const key = normName(name);
    if (modelMap[key]) continue;
    const entry = index.get(`data/model/${key}`);
    if (!entry) { modelMissing++; continue; }
    const bytes = extractFile(grf, entry);
    modelMap[key] = "../" + store("_m", "rsm", bytes);
    try {
      for (const tex of parseRsmTextures(bytes)) textureNames.add(normName(tex));
    } catch (err) {
      console.warn(`  ! ${map}: RSM texture parse failed for ${key}: ${err.message}`);
    }
  }

  const textureMap = {}; // normName -> ../_t/<hash>.png
  let texMissing = 0;
  let texFailed = 0;
  for (const key of textureNames) {
    if (!key) continue;
    const entry = index.get(`data/texture/${key}`);
    if (!entry) { texMissing++; continue; }
    const png = effectTextureToPng(extractFile(grf, entry), key);
    if (!png) { texFailed++; continue; }
    textureMap[key] = "../" + store("_t", "png", png);
  }

  // Animated water: the 32 JPG frames for this map's water type, served as-is.
  const waterFrames = [];
  for (let n = 0; n < 32; n++) {
    const nn = String(n).padStart(2, "0");
    const entry = index.get(`data/texture/워터/water${waterType}${nn}.jpg`);
    if (!entry) continue;
    waterFrames.push("../" + store("_w", "jpg", extractFile(grf, entry)));
  }

  const manifest = {
    map,
    files: { gat: `${map}.gat`, gnd: `${map}.gnd`, rsw: `${map}.rsw` },
    models: modelMap,
    textures: textureMap,
    water: { type: waterType, frames: waterFrames },
    ui,
  };
  // Per-map fog (from data/fogparametertable.txt) — only present for maps with a
  // fog row; omitted otherwise. The .rsw carries no fog data of its own.
  const fog = fogTable && fogTable.get(map);
  if (fog) manifest.fog = fog;

  // In-world effects: one entry per placed type-4 object. Four renderable kinds:
  //  - STR effects (`str` = the id's deduped set of /effects/<key>/ bundles built
  //    by --effects); positions are NOT deduped — the client proximity-culls.
  //  - Sprite effects (`sprite` = a /effects/sprites/<key>/ bundle, also built by
  //    --effects): EF_TORCH/EF_SMOKE/EF_BANJJAKII, played from .spr frames.
  //  - Procedural FUNC effects (EF_FIREFLY): just id + pos + param, no asset.
  //  - Parametric emitters (EF_EMITTER/ANIMATED_EMITTER/MAGIC_FLOOR): not .str —
  //    each placement is matched by horizontal (X/Z) position to its EffectTool
  //    lub entry, whose particle spec is baked inline as `emitter` (texture
  //    rewritten into the shared _t store). Any other id (the hardcoded classic
  //    light/pillar effects) is skipped — the client has no data we can ship.
  const hasEmitters = rswEffects.some((e) => EMITTER_EFFECT_IDS.has(e.id));
  const emitterEntries = hasEmitters ? readMapEmitters(grf, index, map) : [];
  const emitterTexCache = new Map(); // normalized texture name -> ../_t/<hash>.png | null
  // Resolve an emitter texture (e.g. "effect\\smoke2.bmp" / "smoke2.bmp") into the
  // shared _t store, relative to data/texture/effect/ (then data/texture/).
  const resolveEmitterTexture = (texName) => {
    const key = normName(texName);
    if (emitterTexCache.has(key)) return emitterTexCache.get(key);
    const rel = key.replace(/^effect\//, "");
    const ent = index.get(`data/texture/effect/${rel}`) || index.get(`data/texture/${rel}`);
    let served = null;
    if (ent) {
      const png = effectTextureToPng(extractFile(grf, ent), rel);
      if (png) served = "../" + store("_t", "png", png);
    }
    emitterTexCache.set(key, served);
    return served;
  };
  // Nearest lub entry to a placement by X/Z (.rsw pos is ÷5; lub pos is raw), within
  // 5 world units. The lub `pos` is dropped from the baked spec (the placement's own
  // `pos` is authoritative and scene-consistent); `texture` is rewritten to a path.
  const matchEmitter = (e) => {
    const rx = e.pos[0] * 5, rz = e.pos[2] * 5;
    let best = null, bestD = Infinity;
    for (const rec of emitterEntries) {
      const dx = rec.pos[0] - rx, dz = rec.pos[2] - rz;
      const d = dx * dx + dz * dz;
      if (d < bestD) { bestD = d; best = rec; }
    }
    if (!best || bestD > 25) return null;
    const { pos, ...spec } = best;
    if (typeof spec.texture === "string") spec.texture = resolveEmitterTexture(spec.texture);
    return spec;
  };

  const mapEffects = [];
  let emitterMissed = 0;
  for (const e of rswEffects) {
    const refs = effectStrRefs(e.id);
    if (refs.length) {
      mapEffects.push({ id: e.id, pos: e.pos, str: refs.map((r) => r.key), delay: e.delay, param: e.param });
      continue;
    }
    // Sprite effects (EF_TORCH/EF_SMOKE/EF_BANJJAKII): `sprite` = the bundle key
    // served under /effects/sprites/<key>/ (built once by --effects).
    const sprite = SPRITE_EFFECT_TABLE[e.id];
    if (sprite) {
      mapEffects.push({ id: e.id, pos: e.pos, sprite: sprite.key, delay: e.delay, param: e.param });
      continue;
    }
    // Procedural FUNC effects (EF_FIREFLY): no asset — the client renders them
    // from the id + position alone.
    if (FUNC_EFFECT_IDS.has(e.id)) {
      mapEffects.push({ id: e.id, pos: e.pos, delay: e.delay, param: e.param });
      continue;
    }
    if (EMITTER_EFFECT_IDS.has(e.id)) {
      const spec = matchEmitter(e);
      if (spec) mapEffects.push({ id: e.id, pos: e.pos, delay: e.delay, param: e.param, emitter: spec });
      else emitterMissed++;
    }
  }
  if (mapEffects.length) manifest.effects = mapEffects;

  writeFileSync(join(mapDir, "manifest.json"), JSON.stringify(manifest));

  return {
    models: Object.keys(modelMap).length,
    modelTotal: modelNames.length,
    modelMissing,
    textures: Object.keys(textureMap).length,
    textureTotal: textureNames.size,
    texMissing,
    texFailed,
    waterType,
    waterFrames: waterFrames.length,
    fog: !!fog,
    effects: mapEffects.length,
    emitters: mapEffects.filter((e) => e.emitter).length,
    emitterMissed,
  };
}

// Parse a fog colour token → [r, g, b] in 0..1, or null if unparseable. The
// official table stores it as a packed "0xAARRGGBB" D3DCOLOR (leading alpha byte
// then big-endian RGB); we drop the alpha and keep the low three bytes (R = high
// byte) ÷ 255. A bare 6-digit "RRGGBB" (no 0x) is accepted too.
function parseFogColor(tok) {
  let h = tok.trim().replace(/^0x/i, "");
  if (!/^[0-9a-f]+$/i.test(h)) return null;
  if (h.length === 8) h = h.slice(2); // drop the leading alpha byte of AARRGGBB
  if (h.length !== 6) return null;
  const v = parseInt(h, 16);
  return [((v >> 16) & 0xff) / 255, ((v >> 8) & 0xff) / 255, (v & 0xff) / 255];
}

// Parse data/fogparametertable.txt → Map(mapName → fog block). Each record is the
// five "#"-terminated fields  <mapname># <near># <far># <colorHex># <factor>#  —
// the official client puts each field on its own line, so we tokenize on "#"
// across newlines (which also handles a single-line-per-map layout). Comments
// start with "//". The map key may carry a .rsw/.gat/.gnd suffix — stripped and
// lowercased to match the manifest key. near/far/factor are raw floats (the
// client multiplies near/far by 240 itself); colorHex → three 0..1 RGB floats.
// The table is ASCII in its data fields (EUC-KR only in comments), so latin1
// decoding is safe for the rows.
export function parseFogTable(bytes) {
  const tokens = Buffer.from(bytes)
    .toString("latin1")
    .split(/\r?\n/)
    .map((l) => l.replace(/\/\/.*$/, "")) // strip whole-line and trailing comments
    .join("\n")
    .split("#")
    .map((t) => t.trim())
    .filter((t) => t.length > 0);

  const fog = new Map();
  // Records are exactly five fields; grouping positionally stays aligned even if
  // an individual record is malformed (we just skip that group).
  for (let i = 0; i + 4 < tokens.length; i += 5) {
    const name = tokens[i].replace(/\.(rsw|gat|gnd)$/i, "").toLowerCase();
    const near = parseFloat(tokens[i + 1]);
    const far = parseFloat(tokens[i + 2]);
    const color = parseFogColor(tokens[i + 3]);
    const factor = parseFloat(tokens[i + 4]);
    if (!name || !color || !Number.isFinite(near) || !Number.isFinite(far) || !Number.isFinite(factor)) continue;
    fog.set(name, { near, far, color, factor });
  }
  return fog;
}

function extractMaps(grfPath, outBase, args) {
  const grf = openGrf(grfPath);
  try {
    const root = resolve(outBase);
    const single = args.map ? args.map.toLowerCase() : null;

    // A full run rebuilds the whole tree (and its shared stores) deterministically;
    // a single --map run leaves the existing tree in place and just refreshes that
    // map plus any new shared blobs.
    if (!single) rmSync(root, { recursive: true, force: true });
    mkdirSync(root, { recursive: true });

    const store = makeBlobStore(root);

    console.error("Indexing GRF entries…");
    const index = buildEntryIndex(grf);

    console.error("Extracting shared cursor/grid UI…");
    const ui = extractMapUi(grf, index, store);

    // Per-map fog table (folded into each manifest like the ui block).
    const fogEntry = index.get("data/fogparametertable.txt");
    const fogTable = fogEntry ? parseFogTable(extractFile(grf, fogEntry)) : new Map();
    if (fogEntry) console.error(`fogparametertable.txt: ${fogTable.size} map fog entries`);
    else console.warn("  ! data/fogparametertable.txt not found in GRF — manifests will omit fog");

    const names = single ? [single] : listMapNames(grf);
    console.error(`Extracting ${names.length} map${names.length === 1 ? "" : "s"} → ${root}`);

    const extracted = [];
    let skipped = 0;
    let foggy = 0;
    let effecty = 0;
    for (const map of names) {
      try {
        const r = extractOneMap(grf, index, map, root, store, ui, fogTable);
        if (r.skipped) { skipped++; console.warn(`  - skip ${map} (${r.skipped})`); continue; }
        extracted.push(map);
        if (r.fog) foggy++;
        if (r.effects) effecty++;
        if (single || extracted.length % 25 === 0) {
          console.error(
            `  ✓ ${map}: ${r.models}/${r.modelTotal} models, ${r.textures}/${r.textureTotal} textures, ` +
              `water ${r.waterType} (${r.waterFrames}/32)` +
              (r.effects ? `, ${r.effects} effects` : "") +
              (extracted.length % 25 === 0 && !single ? `  [${extracted.length}/${names.length}]` : ""),
          );
        }
      } catch (err) {
        skipped++;
        console.warn(`  ! ${map}: ${err.message}`);
      }
    }

    // index.json lists every map. For a single --map run, merge into the existing
    // index rather than clobbering the full catalogue.
    let allMaps = extracted;
    if (single) {
      try {
        const prev = JSON.parse(readFileSync(join(root, "index.json"), "utf8"));
        allMaps = [...new Set([...(prev.maps || []), ...extracted])];
      } catch {
        // no prior index — start fresh
      }
    }
    allMaps.sort();
    writeFileSync(join(root, "index.json"), JSON.stringify({ maps: allMaps }));

    console.error(
      `\nMaps → ${root}\n` +
        `  extracted: ${extracted.length}\n` +
        `  skipped:   ${skipped}\n` +
        `  with fog:  ${foggy}\n` +
        `  with effects: ${effecty}\n` +
        `  index.json: ${allMaps.length} maps`,
    );
  } finally {
    closeGrf(grf);
  }
}

// ---------------------------------------------------------------------------
// BGM extraction — per-map background music.
//
// The client maps each world map to a BGM track in data/mp3nametable.txt (lines
// of the form "<map>.rsw#bgm\\<file>.mp3#"); the .mp3 files themselves are loose
// on disk in the client's BGM/ folder, not inside the GRF. We parse the table,
// copy every *referenced* track once (tracks are uniquely numbered, so the
// basename de-duplicates naturally), and emit:
//
//   <out>/<file>.mp3     each referenced track (copied verbatim)
//   <out>/index.json     { maps: { "<map>": "<file>.mp3", … } }
//
// The gateway serves <out>/ at /bgm/ (/bgm/index.json + /bgm/<file>.mp3).
// ---------------------------------------------------------------------------

// Parse data/mp3nametable.txt → Map(mapName → mp3 basename). Comment lines start
// with "//"; data lines are "<map>.rsw#bgm\<file>.mp3#" (the path separator and
// trailing junk after the closing "#" vary, so we match leniently). The table is
// EUC-KR in its comments but pure ASCII in the data fields.
function parseMp3NameTable(bytes) {
  const text = Buffer.from(bytes).toString("latin1");
  const re = /^([A-Za-z0-9_@-]+)\.rsw#\s*bgm[\\/]+([^#]+?\.mp3)\s*#/i;
  const map = new Map();
  for (let line of text.split(/\r?\n/)) {
    line = line.trim();
    if (!line || line.startsWith("//")) continue;
    const m = re.exec(line);
    if (!m) continue;
    const name = m[1].toLowerCase();
    // The mp3 field may carry a sub-path; keep only the basename.
    const file = m[2].replace(/^.*[\\/]/, "").toLowerCase();
    map.set(name, file);
  }
  return map;
}

function extractBgm(grfPath, outBase, args) {
  const grf = openGrf(grfPath);
  try {
    const root = resolve(outBase);

    const entry = findBestEntry(grf, normalize("data/mp3nametable.txt"));
    if (!entry) throw new Error("data/mp3nametable.txt not found in GRF");
    const mapToFile = parseMp3NameTable(extractFile(grf, entry));
    console.error(`mp3nametable.txt: ${mapToFile.size} map→bgm mappings`);

    // BGM tracks live next to the GRF (client BGM/ folder), not in the GRF.
    const srcDir = args.bgmsrc ? resolve(args.bgmsrc) : join(dirname(resolve(grfPath)), "BGM");
    if (!existsSync(srcDir)) throw new Error(`BGM source dir not found: ${srcDir} (pass --bgmsrc <dir>)`);
    // Case-insensitive index of the on-disk filenames (the table is lowercase).
    const onDisk = new Map();
    for (const f of readdirSync(srcDir)) onDisk.set(f.toLowerCase(), f);

    rmSync(root, { recursive: true, force: true });
    mkdirSync(root, { recursive: true });

    const maps = {};
    const copied = new Set();
    const missing = new Set();
    for (const [name, file] of mapToFile) {
      const actual = onDisk.get(file);
      if (!actual) { missing.add(file); continue; }
      if (!copied.has(file)) {
        copyFileSync(join(srcDir, actual), join(root, file));
        copied.add(file);
      }
      maps[name] = file;
    }

    const sorted = Object.fromEntries(Object.keys(maps).sort().map((k) => [k, maps[k]]));
    writeFileSync(join(root, "index.json"), JSON.stringify({ maps: sorted }));

    console.error(
      `\nBGM → ${root}\n` +
        `  maps mapped: ${Object.keys(maps).length}\n` +
        `  tracks copied: ${copied.size} (${[...missing].length} referenced track(s) missing on disk)\n` +
        `  index.json: ${Object.keys(sorted).length} maps`,
    );
    if (missing.size) console.warn(`  ! missing tracks: ${[...missing].sort().join(", ")}`);
  } finally {
    closeGrf(grf);
  }
}

// ---------------------------------------------------------------------------
// Sound extraction — skill/effect/monster sound effects (data/wav/**).
//
// The /effect/table rows reference sounds by a `wav` field: a GRF path relative
// to data/wav/ WITHOUT the .wav extension (e.g. "effect/ef_portal" ->
// data/wav/effect/ef_portal.wav; the bare "_heal_effect" -> data/wav/_heal_effect.wav).
// The client asks the gateway for each name it finds there, so we extract the
// entire data/wav/ tree — every name the table can ever hold is then present —
// mirroring the GRF layout under <out>/:
//
//   <out>/effect/ef_portal.wav
//   <out>/_heal_effect.wav
//   <out>/index.json     { count, names: [ "effect/ef_portal", "_heal_effect", … ] }
//
// RO wavs are almost all standard PCM (22050 Hz mono 16-bit), directly
// browser-playable, and are copied verbatim. The handful stored as MS ADPCM
// (format 0x0002) or IMA ADPCM (0x0011) — which some browsers can't decode — are
// transcoded to 16-bit PCM WAV so every served sound plays in Chrome/Firefox/
// Safari. The output is thus uniformly PCM WAV (one Content-Type, audio/wav).
//
// Names are lowercased on write so the gateway's /effect/sound lookup is a plain
// case-normalized path join (the table's `wav` values are all lowercase ASCII;
// the GRF stores mixed casing). The gateway serves <out>/ at /effect/sound.
// ---------------------------------------------------------------------------

// Parse a RIFF/WAVE buffer to { fmt, data } (byte ranges), or null if it isn't a
// WAVE we recognize. Chunk bodies are word-aligned (a padding byte follows an
// odd-length body), per the RIFF spec.
export function parseWav(b) {
  if (b.length < 12 || b.toString("ascii", 0, 4) !== "RIFF" || b.toString("ascii", 8, 12) !== "WAVE") {
    return null;
  }
  let o = 12;
  let fmt = null;
  let data = null;
  while (o + 8 <= b.length) {
    const id = b.toString("ascii", o, o + 4);
    const sz = b.readUInt32LE(o + 4);
    const body = o + 8;
    if (id === "fmt " && sz >= 16 && body + 16 <= b.length) {
      fmt = {
        audioFormat: b.readUInt16LE(body),
        channels: b.readUInt16LE(body + 2),
        sampleRate: b.readUInt32LE(body + 4),
        blockAlign: b.readUInt16LE(body + 12),
        bits: b.readUInt16LE(body + 14),
        // Everything after the 16-byte core (cbSize + format-specific extra).
        ext: sz > 16 ? b.subarray(body + 16, Math.min(body + sz, b.length)) : Buffer.alloc(0),
      };
    } else if (id === "data") {
      data = { offset: body, len: Math.min(sz, b.length - body) };
    }
    o = body + sz + (sz & 1);
  }
  if (!fmt || !data) return null;
  return { fmt, data };
}

// Wrap raw little-endian PCM samples in a canonical 44-byte WAV header.
export function encodeWavPcm(pcm, channels, sampleRate, bits) {
  const blockAlign = channels * (bits >> 3);
  const h = Buffer.alloc(44);
  h.write("RIFF", 0, "ascii");
  h.writeUInt32LE(36 + pcm.length, 4);
  h.write("WAVE", 8, "ascii");
  h.write("fmt ", 12, "ascii");
  h.writeUInt32LE(16, 16);
  h.writeUInt16LE(1, 20); // PCM
  h.writeUInt16LE(channels, 22);
  h.writeUInt32LE(sampleRate, 24);
  h.writeUInt32LE(sampleRate * blockAlign, 28); // byte rate
  h.writeUInt16LE(blockAlign, 32);
  h.writeUInt16LE(bits, 34);
  h.write("data", 36, "ascii");
  h.writeUInt32LE(pcm.length, 40);
  return Buffer.concat([h, pcm]);
}

const clampS16 = (v) => (v < -32768 ? -32768 : v > 32767 ? 32767 : v);

// MS ADPCM (WAVE format 0x0002) → 16-bit PCM. Default coefficient/adaptation
// tables per the Microsoft spec; the coefficient set may be overridden in the fmt
// chunk's extra bytes (cbSize, wSamplesPerBlock, wNumCoef, aCoef[]). Decodes both
// mono and (interleaved-nibble) stereo, though RO sounds are mono.
const MS_ADAPT = [230, 230, 230, 230, 307, 409, 512, 614, 768, 614, 512, 409, 307, 230, 230, 230];
const MS_COEF1 = [256, 512, 0, 192, 240, 460, 392];
const MS_COEF2 = [0, -256, 0, 64, 0, -208, -232];

function decodeMsAdpcm(b, fmt, data) {
  const ch = fmt.channels;
  const blockAlign = fmt.blockAlign;
  if (ch < 1 || blockAlign < 7 * ch) return null;
  let coef1 = MS_COEF1;
  let coef2 = MS_COEF2;
  if (fmt.ext.length >= 6) {
    const numCoef = fmt.ext.readUInt16LE(4);
    if (numCoef > 0 && fmt.ext.length >= 6 + numCoef * 4) {
      coef1 = [];
      coef2 = [];
      for (let i = 0; i < numCoef; i++) {
        coef1.push(fmt.ext.readInt16LE(6 + i * 4));
        coef2.push(fmt.ext.readInt16LE(6 + i * 4 + 2));
      }
    }
  }
  const out = [];
  const end = data.offset + data.len;
  for (let p = data.offset; p + blockAlign <= end; p += blockAlign) {
    const blk = b.subarray(p, p + blockAlign);
    let bp = 0;
    const pred = [];
    const delta = [];
    const s1 = [];
    const s2 = [];
    let ok = true;
    for (let c = 0; c < ch; c++) {
      const idx = blk[bp++];
      if (idx >= coef1.length) { ok = false; break; }
      pred[c] = idx;
    }
    if (!ok) continue;
    for (let c = 0; c < ch; c++) { delta[c] = blk.readInt16LE(bp); bp += 2; }
    for (let c = 0; c < ch; c++) { s1[c] = blk.readInt16LE(bp); bp += 2; }
    for (let c = 0; c < ch; c++) { s2[c] = blk.readInt16LE(bp); bp += 2; }
    // The block header carries the first two samples (sample2 then sample1).
    for (let c = 0; c < ch; c++) out.push(s2[c]);
    for (let c = 0; c < ch; c++) out.push(s1[c]);
    let nibIdx = 0;
    while (bp < blk.length) {
      const byte = blk[bp++];
      for (const nib of [byte >> 4, byte & 0x0f]) {
        const c = nibIdx % ch;
        const signed = nib >= 8 ? nib - 16 : nib;
        let predicted = (s1[c] * coef1[pred[c]] + s2[c] * coef2[pred[c]]) >> 8;
        predicted = clampS16(predicted + signed * delta[c]);
        out.push(predicted);
        s2[c] = s1[c];
        s1[c] = predicted;
        delta[c] = (MS_ADAPT[nib] * delta[c]) >> 8;
        if (delta[c] < 16) delta[c] = 16;
        nibIdx++;
      }
    }
  }
  return samplesToPcm(out);
}

// IMA/DVI ADPCM (WAVE format 0x0011) → 16-bit PCM. Standard step/index tables;
// each block starts with a per-channel header (predictor i16, step index u8,
// reserved u8) then 4-byte words decoded low-nibble first, round-robin per
// channel for stereo.
const IMA_INDEX = [-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8];
// prettier-ignore
const IMA_STEP = [
  7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
  50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230,
  253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963,
  1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
  3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487,
  12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767,
];

function imaNibble(nib, st) {
  const step = IMA_STEP[st.index];
  let diff = step >> 3;
  if (nib & 4) diff += step;
  if (nib & 2) diff += step >> 1;
  if (nib & 1) diff += step >> 2;
  st.predictor = clampS16(st.predictor + (nib & 8 ? -diff : diff));
  st.index += IMA_INDEX[nib];
  if (st.index < 0) st.index = 0;
  else if (st.index > 88) st.index = 88;
  return st.predictor;
}

export function decodeImaAdpcm(b, fmt, data) {
  const ch = fmt.channels;
  const blockAlign = fmt.blockAlign;
  if (ch < 1 || blockAlign < 4 * ch) return null;
  const out = [];
  const end = data.offset + data.len;
  for (let p = data.offset; p + blockAlign <= end; p += blockAlign) {
    const blk = b.subarray(p, p + blockAlign);
    let bp = 0;
    const st = [];
    const chOut = [];
    for (let c = 0; c < ch; c++) {
      const predictor = blk.readInt16LE(bp);
      let index = blk[bp + 2];
      if (index > 88) index = 88;
      bp += 4;
      st[c] = { predictor, index };
      chOut[c] = [predictor];
    }
    while (bp + 4 * ch <= blk.length) {
      for (let c = 0; c < ch; c++) {
        for (let k = 0; k < 4; k++) {
          const byte = blk[bp + k];
          chOut[c].push(imaNibble(byte & 0x0f, st[c]));
          chOut[c].push(imaNibble(byte >> 4, st[c]));
        }
        bp += 4;
      }
    }
    const n = chOut[0].length;
    for (let i = 0; i < n; i++) for (let c = 0; c < ch; c++) out.push(chOut[c][i] ?? 0);
  }
  return samplesToPcm(out);
}

function samplesToPcm(samples) {
  const pcm = Buffer.alloc(samples.length * 2);
  for (let i = 0; i < samples.length; i++) pcm.writeInt16LE(clampS16(samples[i]), i * 2);
  return pcm;
}

// Return a browser-playable WAV for a raw GRF wav buffer. Standard PCM (8/16-bit)
// is returned verbatim; MS/IMA ADPCM is transcoded to 16-bit PCM. Returns
// { bytes, format, transcoded } — or null if it can't be made playable (caller
// skips it). `format` is the source WAVE format tag, for the histogram.
export function toPlayableWav(raw) {
  const b = Buffer.from(raw.buffer, raw.byteOffset, raw.byteLength);
  const parsed = parseWav(b);
  if (!parsed) return null;
  const { fmt } = parsed;
  // Standard PCM (1) and IEEE float (3) are decoded natively by browsers.
  if ((fmt.audioFormat === 1 || fmt.audioFormat === 3) && (fmt.bits === 8 || fmt.bits === 16 || fmt.bits === 24 || fmt.bits === 32)) {
    return { bytes: b, format: fmt.audioFormat, transcoded: false };
  }
  // Transcode the two ADPCM encodings some browsers can't decode. Anything else
  // can't be made uniformly playable, so skip it (never serve unplayable bytes) —
  // the survey found no such format in the current client.
  let pcm = null;
  if (fmt.audioFormat === 2) pcm = decodeMsAdpcm(b, fmt, parsed.data);
  else if (fmt.audioFormat === 17) pcm = decodeImaAdpcm(b, fmt, parsed.data);
  if (!pcm) return null;
  return {
    bytes: encodeWavPcm(pcm, fmt.channels, fmt.sampleRate, 16),
    format: fmt.audioFormat,
    transcoded: true,
  };
}

function extractSounds(grfPath, outBase, args) {
  const grf = openGrf(grfPath);
  try {
    const root = resolve(outBase);

    // Collect every data/wav/** entry, de-duplicating patch-layered copies of the
    // same logical path by keeping the largest (the complete copy — same rule
    // findBestEntry uses). Key/rel path is normalized + lowercased.
    const best = new Map(); // rel (under data/wav/, lowercased) -> entry
    for (const entry of grf.files) {
      if (!(entry.flags & 0x01)) continue;
      const norm = normalize(entry.filename); // lowercased, forward slashes
      if (!norm.startsWith("data/wav/")) continue;
      const rel = norm.slice("data/wav/".length);
      if (!rel || !rel.endsWith(".wav") || rel.includes("..")) continue;
      const cur = best.get(rel);
      if (!cur || entry.uncompSize > cur.uncompSize) best.set(rel, entry);
    }

    rmSync(root, { recursive: true, force: true });
    mkdirSync(root, { recursive: true });

    const names = [];
    const formatHist = {};
    let written = 0;
    let transcoded = 0;
    let skipped = 0; // unparseable/corrupt or an unhandled format (see toPlayableWav)
    for (const [rel, entry] of best) {
      let playable;
      try {
        playable = toPlayableWav(extractFile(grf, entry));
      } catch {
        playable = null;
      }
      if (!playable) { skipped++; continue; }
      formatHist[playable.format] = (formatHist[playable.format] || 0) + 1;
      if (playable.transcoded) transcoded++;

      const dest = join(root, rel);
      mkdirSync(dirname(dest), { recursive: true });
      writeFileSync(dest, playable.bytes);
      names.push(rel.slice(0, -".wav".length)); // request token (no extension)
      written++;
    }

    names.sort();
    writeFileSync(join(root, "index.json"), JSON.stringify({ count: names.length, names }));

    const fmtNames = { 1: "PCM", 2: "MS-ADPCM", 3: "float", 17: "IMA-ADPCM" };
    const hist = Object.keys(formatHist)
      .sort((a, b) => a - b)
      .map((f) => `${fmtNames[f] || `0x${Number(f).toString(16)}`}=${formatHist[f]}`)
      .join(", ");
    console.error(
      `\nsounds → ${root}\n` +
        `  wav entries in GRF (deduped): ${best.size}\n` +
        `  written: ${written} (${transcoded} transcoded from ADPCM), skipped: ${skipped}\n` +
        `  source formats: ${hist}\n` +
        `  index.json: ${names.length} names`,
    );
  } finally {
    closeGrf(grf);
  }
}

// ---------------------------------------------------------------------------
// Monster id universe (--mobids)
// ---------------------------------------------------------------------------

// The client carries no monster stats, but it does carry the full id → AEGIS
// name table, and that is the one thing the RagnaPlace API can't give us: it has
// no bulk mob list (its search caps at 20 pages × 20 = 400 rows), only
// /v1/<gateway>/mob/<id>. So tools/scrape-mobs.mjs walks this list and lets the
// API 404 the ids that aren't monsters.
//
// Note the *datainfo* copy, not `lua files/npcidentity.lub` — the latter is a
// stale subset that stops at id 10203 and so misses every 20000+ mob.
const NPCIDENTITY_PATH = "data/luafiles514/lua files/datainfo/npcidentity.lub";
// Below this the table is player jobs and NPC sprites, never monsters.
const MOB_ID_MIN = 1000;

function extractMobIds(grfPath, outFile) {
  const grf = openGrf(grfPath);
  try {
    const entry = findBestEntry(grf, normalize(NPCIDENTITY_PATH));
    if (!entry) {
      console.error(`Not found in GRF: ${NPCIDENTITY_PATH}`);
      process.exit(1);
    }
    const jobtbl = runChunk(extractFile(grf, entry)).get("jobtbl");
    if (!(jobtbl instanceof LuaTable)) {
      console.error(`${NPCIDENTITY_PATH} defined no jobtbl`);
      process.exit(1);
    }

    const byId = new Map(); // id -> AEGIS name (first name wins; ids are aliased)
    let belowMin = 0;
    let aliases = 0;
    for (const [key, value] of jobtbl.map) {
      if (typeof key !== "string" || typeof value !== "number" || !Number.isInteger(value)) continue;
      if (value < MOB_ID_MIN) { belowMin++; continue; }
      const aegis = key.startsWith("JT_") ? key.slice(3) : key;
      if (byId.has(value)) { aliases++; continue; }
      byId.set(value, aegis);
    }

    const mobs = [...byId.keys()].sort((a, b) => a - b).map((id) => ({ id, aegisId: byId.get(id) }));
    const dest = resolve(outFile);
    mkdirSync(dirname(dest), { recursive: true });
    writeFileSync(dest, `${JSON.stringify({ source: NPCIDENTITY_PATH, count: mobs.length, mobs }, null, 2)}\n`);

    console.error(
      `\nmob ids → ${dest}\n` +
        `  jobtbl entries: ${byId.size + belowMin + aliases}\n` +
        `  candidates (id >= ${MOB_ID_MIN}): ${mobs.length} (${mobs[0]?.id}..${mobs[mobs.length - 1]?.id})\n` +
        `  skipped: ${belowMin} below ${MOB_ID_MIN}, ${aliases} aliased to an id already seen`,
    );
  } finally {
    closeGrf(grf);
  }
}

// ---------------------------------------------------------------------------
// Raw data tables (--raw)
// ---------------------------------------------------------------------------

// The client data tables other projects used to extract for themselves. Each of
// latam-ro-calc, latamvisuais and ragreplaystats carried its own fork of the GRF
// reader and the Lua VM to build these; now they download the JSON from
// /raw/<name>.json and reshape it locally.
//
// These files are a *faithful projection of the client*, deliberately not a
// curated one. Consumer-specific overrides (ragreplaystats' JOB_NAME_OVERRIDE
// and FOOD_STATUS_NAMES, its `[3]` slot suffix, latam-ro-calc's slot bitmask)
// stay in each consumer's transform, so this stays a single unopinionated
// upstream and each project keeps its exact existing output. The one exception
// is skills.json naming the skill ids the client leaves unnamed, which every
// replay consumer needs identically — see UNNAMED_SKILL_NAMES.
//
// Everything is written compact: unlike mobs.json these are never committed, so
// there is no diff to keep readable and the bytes go over the wire on every
// consumer sync.

const RAW_ITEMMOVE_PATH = "data/itemmoveinfov5.txt";
const RAW_MSGSTRING_PATH = "data/msgstringtable_ml.csv";
const RAW_LUB_PATHS = {
  pcidentity: "data/luafiles514/lua files/admin/pcidentity.lub",
  pcjobnamegender: "data/luafiles514/lua files/datainfo/pcjobnamegender.lub",
  skillid: "data/luafiles514/lua files/skillinfoz/skillid.lub",
  skillinfolist: "data/luafiles514/lua files/skillinfoz/skillinfolist_ptbr.lub",
  // The client splits this table in two: _data.lub holds every skill's numbers
  // and _ptBR.lub above only its names. The un-suffixed SkillInfoList.lub is
  // still shipped and still parses, but it is a stale copy of the merge — it is
  // one skill (5383) short of both of these, so the numbers come from _data.
  skillinfodata: "data/luafiles514/lua files/skillinfoz/skillinfolist_data.lub",
  // Full path, deliberately: the GRF also ships this file under data/spanish/
  // and data/english/, and the Spanish copy is the LARGEST of the three — so a
  // suffix-only want ("luafiles514/lua files/…") would make findBestEntry hand
  // back Spanish tooltips that look perfectly valid until someone reads them.
  skilldescript: "data/luafiles514/lua files/skillinfoz/skilldescript.lub",
  skilldelay: "data/luafiles514/lua files/skillinfoz/skilldelaylist.lub",
  enumvar: "data/luafiles514/lua files/datainfo/enumvar.lub",
  randomopt: "data/luafiles514/lua files/datainfo/addrandomoptionnametable_ptbr.lub",
  efstids: "data/luafiles514/lua files/stateicon/efstids.lub",
  stateiconinfo: "data/luafiles514/lua files/stateicon/stateiconinfo.lub",
  // Box contents. Full path for the same reason as skilldescript: the GRF also
  // ships this table under data/english/ and data/spanish/, and the Spanish copy
  // is the largest of the three, so a suffix-only want would silently win it.
  packageitem: "data/luafiles514/lua files/probabilityinfo/packageitem.lub",
};

// data/itemmoveinfov5.txt lists every item as `<id>\t<move flags...>\t// <AegisName>`;
// the trailing comment is the real item_db aegis name. A few early lines carry a
// generic comment instead (cashitem, Korean text, names with spaces), so keep
// only clean aegis tokens — a bare identifier that looks like a name, not prose.
export function parseAegisMap(text) {
  const map = new Map();
  for (const line of text.split(/\r?\n/)) {
    const m = line.match(/^\s*(\d+)\t.*?\/\/\s*(.+?)\s*$/);
    if (!m) continue;
    const aegis = m[2];
    if (/^[A-Za-z0-9][A-Za-z0-9_]*$/.test(aegis) && (aegis.includes("_") || /[A-Z]/.test(aegis))) {
      map.set(Number(m[1]), aegis);
    }
  }
  return map;
}

// Flatten a compiled Lua 5.1 chunk's constant pool, outermost proto first, without
// executing it. Used only for the two job tables: pcidentity/pcjobnamegender encode
// their mapping as adjacent constants, and reading them positionally is what has
// always produced the consumers' job names — running them through the VM would build
// differently shaped globals and silently change the output.
//
// The bytecode reader is loadChunk's, so there is one Lua loader in this file rather
// than a second one that would have to be fixed twice when a client fork shifts the
// format. Returns null for anything that isn't a chunk we can read.
export function parseLuaConstants(bytes) {
  // loadChunk trusts the endianness byte; these tables are only ever little-endian.
  if (bytes.length < 12 || bytes[6] !== 1) return null;
  let proto;
  try {
    proto = loadChunk(bytes);
  } catch {
    return null;
  }

  const out = [];
  const walk = (p) => {
    for (const k of p.k) {
      if (k === undefined) out.push({ type: "nil" });
      else if (typeof k === "boolean") out.push({ type: "bool", value: k });
      else if (typeof k === "number") out.push({ type: "number", value: k });
      // readString returns null for the empty string; keep it a string either way.
      else out.push({ type: "string", value: k ?? "" });
    }
    for (const nested of p.protos) walk(nested);
  };
  walk(proto);
  return out;
}

// pcidentity.lub: each `JT_<NAME>` string constant is immediately followed by its
// numeric id. First id wins — the table aliases some names.
export function jtIdsFromConstants(consts) {
  const ids = new Map();
  if (!consts) return ids;
  for (let i = 0; i + 1 < consts.length; i++) {
    const a = consts[i];
    const b = consts[i + 1];
    if (a.type === "string" && /^JT_/.test(a.value) && b.type === "number" && !ids.has(a.value)) {
      ids.set(a.value, b.value);
    }
  }
  return ids;
}

// pcjobnamegender.lub: a `JT_<NAME>` constant is followed by its display label —
// the next string that isn't itself a JT_ key or one of the table names the
// chunk also stores as constants.
const JOB_TABLE_CONSTS = new Set(["PCJobNameTableMan", "PCJobNameTableWoman", "pcJobTbl2"]);

export function jobLabelsFromConstants(consts) {
  const labels = new Map();
  if (!consts) return labels;
  for (let i = 0; i < consts.length; i++) {
    const c = consts[i];
    if (c.type !== "string" || !c.value.startsWith("JT_")) continue;
    for (let j = i + 1; j < consts.length; j++) {
      const cc = consts[j];
      if (cc.type !== "string") continue;
      if (cc.value.startsWith("JT_")) break;
      if (JOB_TABLE_CONSTS.has(cc.value)) continue;
      labels.set(c.value, cc.value);
      break;
    }
  }
  return labels;
}

// One record per item. `name` is the bare identified name — the client appends
// the "[3]" slot suffix at display time, and consumers disagree about whether
// they want it, so `slots` stays a separate number. `aegisName` and
// `resourceName` are both kept: consumers that want the item_db name prefer the
// former and fall back to the latter, and folding them here would erase which
// one a row actually came from.
//
// Rows with no display name are kept, with `name: null`. They are ~640 entries
// that still carry a renderable `view`, and dropping them here would lose the
// sprite ids the calculator's paper-doll needs; every consumer that wants named
// items filters on `name` anyway.
//
// `view` is the raw ClassNum. `spriteView` is the view the renderer actually
// draws with: newer costumes ship ClassNum 0 and keep their real view only in
// the client's accessory/robe name tables, so without the recovery a costume
// simulator silently loses them (28 of them in the current client). The two stay
// separate because a consumer that wants the literal client field should not
// have to guess whether a value was recovered.
export function projectItems(tbl, aegisMap = new Map(), views = null, packages = new Map()) {
  const out = [];
  if (!(tbl instanceof LuaTable)) return out;
  for (const [id, entry] of tbl.map) {
    if (typeof id !== "number" || !(entry instanceof LuaTable)) continue;
    // Trimmed: a few client names end in spaces ("Carta Maya Silente       ").
    const name = (decodeClientString(entry.get("identifiedDisplayName")) || "").trim() || null;
    const classNum = entry.get("ClassNum");
    const view = typeof classNum === "number" && classNum > 0 ? Math.round(classNum) : 0;
    const equipSlots = parseSlots(entry.get("identifiedDescriptionName"));
    const resourceName = entry.get("identifiedResourceName");
    const spriteView = view || (views?.resolveView(equipSlots, resourceName) ?? 0);
    out.push({
      id,
      name,
      slots: Number(entry.get("slotCount") || 0),
      aegisName: aegisMap.get(id) ?? null,
      resourceName: decodeClientString(resourceName) || null,
      description: joinDescriptionLines(entry.get("identifiedDescriptionName")),
      view,
      spriteView,
      // The view resolves but nothing is drawn for it: its sprite is blank by
      // design (every .act layer tinted alpha 0), which is how the client says
      // the visual is an effect, and no hat-effect sprite stands behind it that
      // the renderer can composite in its place. Consumers that draw a costume
      // from spriteView have to skip these, or they publish an empty preview —
      // see /effects/index.json for the ones whose effect is a .str we can serve.
      spriteBlank: Boolean(spriteView && views?.drawsNothing(spriteView, equipSlots)),
      viewKind: (spriteView && views?.spriteKind(spriteView, resourceName)) || null,
      equipSlots,
      costume: entry.get("costume") === true,
      // Only a box carries `contains`; every other row omits the key rather
      // than carrying an empty array. 15.4k of the 16.9k items are not boxes,
      // and `"contains":[]` on each of them is 216 KB of nothing in a file
      // consumers pull over the wire. Read it as `item.contains ?? []`.
      ...(packages.get(id) ? { contains: packages.get(id) } : {}),
    });
  }
  return out.sort((a, b) => a.id - b.id);
}

// probabilityinfo/packageitem.lub: what a box can give, keyed by the box's own
// item id. Returns id -> the drop list, so projectItems can hang it off the row
// the box already has instead of publishing a second id-keyed file.
//
// The client stores each drop's display name inline, and we drop it: it is a
// denormalized copy of a name items.json already carries, and a worse one —
// 4776 of the 13111 published rows disagree with items.json, almost all of them
// because the package table bakes in the "[2]" slot suffix that `slots` keeps
// separate, and all 37 drops that no longer have an item row are literally named
// "Unknown Item". Consumers join on `id` the way they do everywhere else.
//
// `prob` is the client's raw weight, passed through unnormalized, because there
// is no single denominator to normalize against: per-group sums across the file
// land on 10000 and 20000 (basis points) for the gacha-style boxes but on 1, 2,
// 3, 10 … for the fixed-contents ones, and 552 groups sum to 0. A consumer that
// wants a percentage sums the weights of one `group` and divides by that.
//
// `group` is the sub-pool: a box rolls each of its groups independently, so a
// box with a group 0 and a group 6 hands out one drop from each.
export function projectPackages(tbl) {
  const out = new Map();
  if (!(tbl instanceof LuaTable)) return out;
  for (const [boxId, pkg] of tbl.map) {
    if (typeof boxId !== "number" || !(pkg instanceof LuaTable)) continue;
    const drops = [];
    for (const row of pkg.map.values()) {
      if (!(row instanceof LuaTable)) continue;
      const id = row.get("id");
      if (typeof id !== "number") continue;
      drops.push({
        id: Math.round(id),
        prob: Number(row.get("prob") || 0),
        group: Number(row.get("group") || 0),
      });
    }
    if (drops.length) out.set(Math.round(boxId), drops);
  }
  return out;
}

// The description is stored as an array of lines; join them and keep the ^RRGGBB
// colour codes, which the consumers' own formats preserve.
function joinDescriptionLines(desc) {
  if (!(desc instanceof LuaTable)) return "";
  const lines = [];
  for (const line of desc.map.values()) {
    if (typeof line === "string") lines.push(decodeClientString(line));
  }
  return lines.join("\n");
}

// The name tables are the same shape — numeric id -> a value the name has to be
// dug out of — so they share one walk and differ only in how deep the name is
// buried. Rows without a name are dropped rather than emitted as null: unlike
// items, an unnamed status/option has nothing else worth publishing. (Skills
// have the same rule but a second column, so they walk this shape themselves.)
function projectNamed(tbl, nameOf) {
  const out = [];
  if (!(tbl instanceof LuaTable)) return out;
  for (const [id, value] of tbl.map) {
    if (typeof id !== "number") continue;
    const name = nameOf(value);
    if (name) out.push({ id: Math.round(id), name });
  }
  return out.sort((a, b) => a.id - b.id);
}

// Skills carry their tooltip in a second table (SKILL_DESCRIPT), keyed by the
// same SKID consts: id -> the array of lines the client stacks in the tooltip.
// It is joined exactly like an item's description — ^RRGGBB colour codes and
// line breaks kept, nothing reflowed — because the pt-BR client text is what
// consumers treat as the source of truth for what a skill actually does.
//
// A skill with no description block keeps `description: null` rather than being
// dropped or emptied: consumers index skills.json by id and expect every named
// skill to stay listed. `delay` and `maxLevel` follow the same rule.
//
// `skid` (SKID const -> id) turns on the one curated part of this table: ids the
// client leaves unnamed get a name from UNNAMED_SKILL_NAMES or from their parent
// skill, and follow-up hits carry `parent` — see resolveUnnamedSkills. Without
// it the projection is the client's names alone.
export function projectSkills(list, descriptions = null, delays = null, info = null, skid = null) {
  const out = [];
  if (!(list instanceof LuaTable)) return out;
  const names = new Map();
  for (const [key, entry] of list.map) {
    if (typeof key !== "number") continue;
    // Trimmed: a handful of client names carry a trailing space ("Lava Quente ").
    const name = entry instanceof LuaTable ? (decodeClientString(entry.get("SkillName")) || "").trim() : "";
    if (name) names.set(Math.round(key), name);
  }
  const { filled, parents } = skid ? resolveUnnamedSkills(skid, names) : { filled: new Map(), parents: new Map() };
  for (const [id, name] of filled) names.set(id, name);

  for (const [id, name] of names) {
    const maxLv = info?.get(id) instanceof LuaTable ? info.get(id).get("MaxLv") : null;
    out.push({
      id,
      name,
      maxLevel: typeof maxLv === "number" ? Math.round(maxLv) : null,
      description: joinDescriptionLines(descriptions?.get(id)) || null,
      delay: skillDelay(delays?.get(id)),
      // Only a follow-up hit carries the key, and only when its parent has a
      // row to point at. Read it as `skill.parent ?? null`.
      ...(names.has(parents.get(id)) ? { parent: parents.get(id) } : {}),
    });
  }
  return out.sort((a, b) => a.id - b.id);
}

// ---------------------------------------------------------------------------
// Skills the client leaves unnamed
// ---------------------------------------------------------------------------
//
// skillid.lub defines ~240 ids that SkillInfoList gives no name, and the server
// uses some of them: a replay carries the explosion of Crimson Arrow under its
// own id, and a monster's NPC_* skills under theirs. Every consumer that shows a
// replay used to print "skill#686" or keep its own table of names, so this is
// the one place /raw deliberately goes beyond the client — and only where the
// client is silent. A name the client ships always wins, the moment a patch
// ships it. The single exception is SAME_NAME_SUFFIXES, which suffixes a client
// name that is word for word another skill's, and stops the moment it isn't.
//
// There are two kinds of fill:
//
// - A follow-up hit is a second id the server sends for a named skill's later
//   hits (WL_TETRAVORTEX_FIRE after WL_TETRAVORTEX). It is named after its
//   parent plus a pt-BR suffix, and carries `parent` so a consumer can count a
//   use once rather than once per packet. The relation is read off the SKID
//   constants, so it holds whether or not the client names the child: a patch
//   that names one changes its name, not what it counts towards.
// - Everything else gets a name from UNNAMED_SKILL_NAMES, written from what the
//   constant plainly says, or copied from the client's own name for the skill
//   it mirrors. Ids whose constant says nothing a name can be built from stay
//   out of skills.json rather than get an invented one.

// Suffixes that mark a follow-up hit when the constant left over is itself a
// skill. Longest first, so _DOUBLE_ATK is not read as _ATK on SU_PICKYPECK_DOUBLE.
const FOLLOW_UP_SUFFIXES = [
  ["_DOUBLE_ATK", "ataque duplo"],
  ["_DOTDAMAGE", "dano contínuo"],
  ["_PLUSATK", "ataque adicional"],
  ["_GROUND", "terra"],
  ["_POISON", "veneno"],
  ["_WATER", "água"],
  ["_MAGIC", "mágico"],
  ["_MELEE", "corpo a corpo"],
  ["_ATK2", "ataque 2"],
  ["_FIRE", "fogo"],
  ["_WIND", "vento"],
  ["_ATK", "ataque"],
];

// Follow-ups the suffix rule gets wrong or cannot see. `parent` is a SKID const
// and replaces the derived parent; `suffix` replaces the derived suffix.
export const FOLLOW_UP_OVERRIDES = {
  // The spheres are summoned by WL_SUMMONFB/BL/WB/STONE, which share no stem
  // with the attacks that release them — and ground pairs with STONE (2229).
  WL_SUMMON_ATK_FIRE: { parent: "WL_SUMMONFB", suffix: "ataque" },
  WL_SUMMON_ATK_WIND: { parent: "WL_SUMMONBL", suffix: "ataque" },
  WL_SUMMON_ATK_WATER: { parent: "WL_SUMMONWB", suffix: "ataque" },
  WL_SUMMON_ATK_GROUND: { parent: "WL_SUMMONSTONE", suffix: "ataque" },
  AG_CRIMSON_ARROW_ATK: { suffix: "explosão" },
  OB_OBOROGENSOU_TRANSITION_ATK: { parent: "OB_OBOROGENSOU", suffix: "ataque de transição" },
  GN_SLINGITEM_RANGEMELEEATK: { parent: "GN_SLINGITEM", suffix: "ataque à distância" },
  GN_FIRE_EXPANSION_SMOKE_POWDER: { parent: "GN_FIRE_EXPANSION", suffix: "pó de fumaça" },
  GN_FIRE_EXPANSION_TEAR_GAS: { parent: "GN_FIRE_EXPANSION", suffix: "gás lacrimogêneo" },
  GN_FIRE_EXPANSION_ACID: { parent: "GN_FIRE_EXPANSION", suffix: "ácido" },
  SR_CRESCENTELBOW_AUTOSPELL: { parent: "SR_CRESCENTELBOW", suffix: "contra-ataque" },
  LG_OVERBRAND_BRANDISH: { parent: "LG_OVERBRAND", suffix: "brandir" },
  // Ignição detonating the Armadilha Aderente traps; replays send it right
  // after an RL_FLICKER use.
  RL_B_FLICKER_ATK: { parent: "RL_FLICKER", suffix: "Armadilha Aderente" },
  // Not in LATAM's skillid.lub at all — see SKID_ADDITIONS.
  AG_DESTRUCTIVE_HURRICANE_BUFF: { parent: "AG_DESTRUCTIVE_HURRICANE", suffix: "bônus" },
};

// Skill ids the server sends that LATAM's skillid.lub does not define, so there
// is no constant to hang a name on. The constants are kRO's, as divine-pride
// lists them. Merged in only while the client defines neither the constant nor
// the id; once it does, its own entry is used.
export const SKID_ADDITIONS = {
  // A self-targeted use right after AG_DESTRUCTIVE_HURRICANE.
  AG_DESTRUCTIVE_HURRICANE_BUFF: 5306,
};

export function withSkidAdditions(skid) {
  const out = new Map(skid);
  const ids = new Set(skid.values());
  for (const [konst, id] of Object.entries(SKID_ADDITIONS)) {
    if (!out.has(konst) && !ids.has(id)) out.set(konst, id);
  }
  return out;
}

// Casts the client names exactly like the skill they are a mode of, so two rows
// would read the same. The server sends the *use* under these ids and the damage
// under the base skill's, so they are not follow-up hits and get no `parent`:
// with one, a consumer would count no uses of the skill at all. They get a
// suffix only while their client name is still the base skill's — a patch that
// gives one a name of its own wins. The suffixes are the tooltip's own words.
export const SAME_NAME_SUFFIXES = {
  NW_THE_VIGILANTE_AT_NIGHT_GUN_GATLING: { of: "NW_THE_VIGILANTE_AT_NIGHT", suffix: "metralhadora" },
  NW_THE_VIGILANTE_AT_NIGHT_GUN_SHOTGUN: { of: "NW_THE_VIGILANTE_AT_NIGHT", suffix: "espingarda" },
};

// Constants the suffix rule matches that are skills in their own right, cast on
// their own: counting them as hits of the stem would erase every use of them.
const NOT_FOLLOW_UPS = new Set(["RK_DRAGONBREATH_WATER"]);

// Names for unnamed ids that are not follow-ups, keyed by SKID const. A string
// is the name; { sameAs } copies the name of another skill (client-named or from
// this table) — the client's own convention for a monster's copy of a player
// skill (NPC_DRAGONBREATH ships as RK_DRAGONBREATH's "Sopro do Dragão") and for
// the `2` variants of one skill (NPC_WIDEBLEEDING2). Neither kind gets `parent`:
// each of these is a cast of its own, and a consumer that folded them into the
// skill they mirror would stop counting them. Element words follow the client's
// descriptions (Sombrio, Maldito, Fantasma — telekinesis is ghost).
export const UNNAMED_SKILL_NAMES = {
  NPC_PIERCINGATT: "Ataque Perfurante",
  NPC_MENTALBREAKER: "Quebra Mental",
  NPC_RANGEATTACK: "Ataque à Distância",
  NPC_ATTRICHANGE: "Mudar Propriedade",
  NPC_CHANGEWATER: "Mudar Propriedade (Água)",
  NPC_CHANGEGROUND: "Mudar Propriedade (Terra)",
  NPC_CHANGEFIRE: "Mudar Propriedade (Fogo)",
  NPC_CHANGEWIND: "Mudar Propriedade (Vento)",
  NPC_CHANGEPOISON: "Mudar Propriedade (Veneno)",
  NPC_CHANGEHOLY: "Mudar Propriedade (Sagrado)",
  NPC_CHANGEDARKNESS: "Mudar Propriedade (Sombrio)",
  NPC_CHANGETELEKINESIS: "Mudar Propriedade (Fantasma)",
  NPC_CHANGEUNDEAD: "Mudar Propriedade (Maldito)",
  NPC_CHANGEUNDEAD2: { sameAs: "NPC_CHANGEUNDEAD" },
  NPC_CRITICALSLASH: "Golpe Crítico",
  NPC_COMBOATTACK: "Ataque Combinado",
  NPC_GUIDEDATTACK: "Ataque Guiado",
  NPC_SELFDESTRUCTION: "Autodestruição",
  NPC_SPLASHATTACK: "Ataque em Área",
  NPC_SUICIDE: "Suicídio",
  NPC_POISON: { sameAs: "TF_POISON" },
  NPC_BLINDATTACK: "Ataque Cegante",
  NPC_SILENCEATTACK: "Ataque Silenciador",
  NPC_STUNATTACK: "Ataque Atordoante",
  NPC_PETRIFYATTACK: "Ataque Petrificante",
  NPC_CURSEATTACK: "Ataque Amaldiçoante",
  NPC_SLEEPATTACK: "Ataque Sonífero",
  NPC_RANDOMATTACK: "Ataque Aleatório",
  NPC_WATERATTACK: "Ataque de Água",
  NPC_GROUNDATTACK: "Ataque de Terra",
  NPC_FIREATTACK: "Ataque de Fogo",
  NPC_WINDATTACK: "Ataque de Vento",
  NPC_POISONATTACK: "Ataque de Veneno",
  NPC_HOLYATTACK: "Ataque Sagrado",
  NPC_DARKNESSATTACK: "Ataque Sombrio",
  NPC_TELEKINESISATTACK: "Ataque Fantasma",
  NPC_UNDEADATTACK: "Ataque Maldito",
  NPC_MAGICALATTACK: "Ataque Mágico",
  NPC_METAMORPHOSIS: "Metamorfose",
  NPC_PROVOCATION: "Provocação",
  NPC_SMOKING: "Fumar",
  NPC_SUMMONSLAVE: "Invocar Servos",
  NPC_CALLSLAVE: "Chamar Servos",
  NPC_SUMMONMONSTER: "Invocar Monstro",
  NPC_EMOTION: "Emoção",
  NPC_EMOTION_ON: "Emoção (ativar)",
  NPC_TRANSFORMATION: "Transformação",
  NPC_BLOODDRAIN: "Drenar Sangue",
  NPC_ENERGYDRAIN: "Drenar Energia",
  NPC_DARKBREATH: "Sopro Sombrio",
  NPC_DARKBLESSING: "Bênção Sombria",
  NPC_BARRIER: "Barreira",
  NPC_LICK: "Lamber",
  NPC_HALLUCINATION: "Alucinação",
  NPC_REBIRTH: "Renascimento",
  NPC_RANDOMMOVE: "Movimento Aleatório",
  NPC_SPEEDUP: "Aumentar Velocidade",
  NPC_REVENGE: "Vingança",
  NPC_DARKCROSS: "Cruz Sombria",
  NPC_GRANDDARKNESS: "Grande Escuridão",
  NPC_DARKSTRIKE: "Golpe Sombrio",
  NPC_DARKTHUNDER: "Trovão Sombrio",
  NPC_STOP: "Parar",
  NPC_WEAPONBRAKER: "Quebrar Arma",
  NPC_ARMORBRAKE: "Quebrar Armadura",
  NPC_HELMBRAKE: "Quebrar Capacete",
  NPC_SHIELDBRAKE: "Quebrar Escudo",
  NPC_POWERUP: "Aumentar Poder",
  NPC_AGIUP: "Aumentar Agilidade",
  NPC_SIEGEMODE: "Modo de Cerco",
  NPC_INVISIBLE: "Invisibilidade",
  NPC_RUN: "Fugir",
  NPC_FIREBREATH: "Sopro de Fogo",
  NPC_ICEBREATH: "Sopro de Gelo",
  NPC_ICEBREATH2: { sameAs: "NPC_ICEBREATH" },
  NPC_THUNDERBREATH: "Sopro do Trovão",
  NPC_ACIDBREATH: "Sopro Ácido",
  NPC_ACIDBREATH2: { sameAs: "NPC_ACIDBREATH" },
  NPC_DARKNESSBREATH: "Sopro das Trevas",
  NPC_BLEEDING: "Sangramento",
  NPC_BLEEDING2: { sameAs: "NPC_BLEEDING" },
  NPC_EXPULSION: "Expulsão",
  NPC_TALK: "Falar",
  NPC_INVINCIBLE: "Invencibilidade",
  NPC_INVINCIBLEOFF: "Invencibilidade (desativar)",
  NPC_ICEEXPLO: "Explosão de Gelo",
  NPC_DANCINGBLADE: "Lâminas Dançantes",
  NPC_DARKPIERCING: "Perfuração Sombria",
  NPC_DEATHSUMMON: "Invocação da Morte",
  NPC_HELLBURNING: "Chamas Infernais",
  NPC_CHEAL: { sameAs: "AB_CHEAL" },
  NPC_VENOMIMPRESS: { sameAs: "GC_VENOMIMPRESS" },
  NPC_PHANTOMTHRUST: { sameAs: "RK_PHANTOMTHRUST" },
  NPC_POISON_BUSTER: { sameAs: "SO_POISON_BUSTER" },
  NPC_WIDEDISPEL: "Desencantar em Área",
  NPC_ALL_STAT_DOWN: "Redução de Atributos",
  // Self-cast by bosses (Eddga, Ifrit, Senhor dos Orcs, Schmidt Corrompido).
  NPC_DAMAGE_HEAL: { sameAs: "AL_HEAL" },
  NPC_GRADUAL_GRAVITY: "Gravidade Gradual",
  NPC_MOVE_COORDINATE: "Mover para Coordenada",
  NPC_RELIEVE_ON: "Alívio (ativar)",
  NPC_RELIEVE_OFF: "Alívio (desativar)",
  NPC_LOCKON_LASER: "Laser de Mira",
  NPC_SEEDTRAP: "Armadilha de Sementes",
  // The client names the `2` variant and not the original.
  NPC_DEADLYCURSE: { sameAs: "NPC_DEADLYCURSE2" },
  NPC_RANDOMBREAK: "Quebra Aleatória",
  // Betelgeuse. The client calls shadow gear "Sombrio".
  NPC_STRIP_SHADOW: "Remover Equipamento Sombrio",
  NPC_CANE_OF_EVIL_EYE: "Bengala do Olho Maligno",
  NPC_CURSE_OF_RED_CUBE: "Maldição do Cubo Vermelho",
  NPC_CURSE_OF_BLUE_CUBE: "Maldição do Cubo Azul",
  NPC_KILLING_AURA: "Aura Assassina",
  // Cast by the plants BO_WOODENWARRIOR, BO_HELLTREE and BO_WOODEN_FAIRY grow,
  // whose client names ("Cultivar Bárbaro" …) supply the qualifier.
  NPC_BO_THROWROCK: "Arremesso de Pedra (Bárbaro)",
  NPC_BO_WOODEN_ATTACK: "Ataque de Madeira (Bárbaro)",
  NPC_BO_HELL_HOWLING: "Uivo Infernal (Árvore Infernal)",
  NPC_BO_HELL_DUSTY: "Poeira Infernal (Árvore Infernal)",
  NPC_BO_FAIRY_DUSTY: "Poeira de Fada (Fada)",
  // Item and event casts of a player skill.
  CASH_BLESSING: { sameAs: "AL_BLESSING" },
  CASH_INCAGI: { sameAs: "AL_INCAGI" },
  CASH_ASSUMPTIO: { sameAs: "HP_ASSUMPTIO" },
  EVT_FULL_THROTTLE: { sameAs: "ALL_FULL_THROTTLE" },
  // Catnip Meteor and Lunatic Carrot Beat, cast while carrying the item that
  // upgrades them.
  SU_CN_METEOR2: { sameAs: "SU_CN_METEOR" },
  SU_LUNATICCARROTBEAT2: { sameAs: "SU_LUNATICCARROTBEAT" },
  // Provocar, cast on oneself.
  SM_SELFPROVOKE: "Provocar (em si mesmo)",
  // What an Elemental Converter or Água Amaldiçoada casts; the level is the
  // element (2 água, 3 terra, 4 fogo, 8 sombrio). Named after the constant and
  // the converters' "Encanta a arma atual…".
  ITEM_ENCHANTARMS: "Encantar Arma",
  ALL_ASSISTANT_VENDING: "Assistente de Comércio",
  ALL_ASSISTANT_BUYING: "Assistente de Compras",
};

// The icon (an item/ BMP basename) every monster skill without its own is drawn
// with — HFLI_SBR44's, see extractIcons.
const MONSTER_SKILL_ICON = "hfli_sbr44";

// Range markers and system slots in SKID. They are never cast, so an unnamed one
// is not worth reporting, nor given an icon.
const SKID_MARKER = /(^|_)(STARTMARK|ENDMARK|START_MARK|BEGIN|START|END|LAST|999)$|^SYS_/;

// SKID const -> id -> parent id, for every follow-up hit the constants describe.
// Also used by --icons, which gives a follow-up with no icon its parent's.
export function skillFollowUps(skid) {
  const parents = new Map();
  for (const [konst, id] of skid) {
    if (NOT_FOLLOW_UPS.has(konst)) continue;
    const override = FOLLOW_UP_OVERRIDES[konst];
    let parent = override?.parent;
    let suffix = override?.suffix;
    if (!parent) {
      for (const [end, label] of FOLLOW_UP_SUFFIXES) {
        if (konst.endsWith(end) && skid.has(konst.slice(0, -end.length))) {
          parent = konst.slice(0, -end.length);
          suffix ??= label;
          break;
        }
      }
    }
    const parentId = skid.get(parent);
    if (parentId !== undefined && parentId !== id && suffix) parents.set(id, { parentId, suffix, konst, parent });
  }
  return parents;
}

// Names for the ids `names` (id -> the client's name) leaves empty, and the
// parent of every follow-up hit. `report` lists what the tables could not use:
// `shadowed`, entries the client now names (safe to delete); `unknown`, entries
// that resolve to nothing (a constant SKID no longer defines, or a sameAs with
// no name to copy); and `unnamed`, ids that neither the client nor this file
// names.
export function resolveUnnamedSkills(skid, names) {
  const filled = new Map();
  const report = { shadowed: [], unknown: [], unnamed: [] };
  const nameOf = (konst) => {
    const id = skid.get(konst);
    return id === undefined ? undefined : (names.get(id) ?? filled.get(id));
  };

  // Plain names first, so a sameAs or a follow-up can build on one of them.
  const entries = Object.entries(UNNAMED_SKILL_NAMES);
  for (const pass of ["string", "object"]) {
    for (const [konst, value] of entries) {
      if (typeof value !== pass) continue;
      const id = skid.get(konst);
      if (id === undefined) {
        if (pass === "string") report.unknown.push(konst);
        continue;
      }
      if (names.has(id)) {
        report.shadowed.push(konst);
        continue;
      }
      const name = typeof value === "string" ? value : nameOf(value.sameAs);
      if (name) filled.set(id, name);
      else report.unknown.push(`${konst} (its sameAs ${value.sameAs} has no name)`);
    }
  }

  const followUps = skillFollowUps(skid);
  const parents = new Map();
  for (const [id, { parentId, suffix, parent }] of followUps) {
    parents.set(id, parentId);
    if (names.has(id) || filled.has(id)) continue;
    const parentName = nameOf(parent);
    if (parentName) filled.set(id, `${parentName} (${suffix})`);
  }
  for (const konst of Object.keys(FOLLOW_UP_OVERRIDES)) {
    if (!skid.has(konst)) report.unknown.push(konst);
  }

  // Filled last and from the client's names only, so the comparison is always
  // against what the client ships.
  for (const [konst, { of, suffix }] of Object.entries(SAME_NAME_SUFFIXES)) {
    const id = skid.get(konst);
    const base = names.get(skid.get(of));
    if (id === undefined || base === undefined) report.unknown.push(konst);
    else if (names.get(id) === base) filled.set(id, `${base} (${suffix})`);
    else if (names.has(id)) report.shadowed.push(konst);
  }

  const seen = new Set();
  for (const [konst, id] of skid) {
    if (seen.has(id) || names.has(id) || filled.has(id)) continue;
    seen.add(id);
    if (!SKID_MARKER.test(konst)) report.unnamed.push(`${id} ${konst}`);
  }
  return { filled, parents, report };
}

// SKILL_DELAY_LIST is what the client's "Conjuração e Espera" window prints: one
// row per skill level, in milliseconds. The client's field names say how each
// number behaves rather than what the window calls it, so both are kept here —
// the window's columns are Fixa, Variável, Pós and Recarga, in this order.
const SKILL_DELAY_FIELDS = {
  castFixed: "SkillCastFixedDelay", // Fixa — cast time no stat/gear reduces
  castVariable: "SkillCastStatDelay", // Variável — the DEX/INT-reducible part
  afterCast: "SkillGlobalPostDelay", // Pós — blocks *every* skill afterwards
  cooldown: "SkillSinglePostDelay", // Recarga — blocks only this skill
};

// The per-level arrays are published exactly as the client stores them: no
// padding invented, no trailing zeros trimmed, index N-1 = level N. Their length
// is *usually* the skill's `maxLevel` (2,733 of the 3,044 columns in the current
// client) but not reliably: 258 are padded past it, and 53 stop short — 52 of
// those holding a single value that plainly means "same at every level", one
// (399 Ataque Vital) five values for ten levels. Normalising them here would
// bake a guess about that last case into every consumer, so the raw shape goes
// out next to `maxLevel` and each consumer clamps and fills as it prefers.
//
// A skill the client gives no timings at all — no row, or a row holding only the
// `SkillFlag` list — gets `delay: null`, not an object of nulls; likewise a
// single missing column stays null rather than becoming `[0]`, so "the client
// says nothing" never reads as "the client says zero". `SkillFlag` itself is
// dropped: its entries are `SKFLAG_*` constants no shipped lua file defines
// (8 skills carry one), so they arrive as nothing this VM can resolve.
function skillDelay(row) {
  if (!(row instanceof LuaTable)) return null;
  const out = {};
  let any = false;
  for (const [key, field] of Object.entries(SKILL_DELAY_FIELDS)) {
    const arr = row.get(field);
    const levels = [];
    if (arr instanceof LuaTable) {
      // Read by index rather than iterating the map: the arrays are contiguous
      // 1..n, and indexing keeps level order independent of insertion order.
      for (let lv = 1; typeof arr.get(lv) === "number"; lv++) levels.push(Math.round(arr.get(lv)));
    }
    out[key] = levels.length ? levels : null;
    if (levels.length) any = true;
  }
  return any ? out : null;
}

// StateIconList entries hold the tooltip under `descript`; descript[1] is the
// title line — either a bare string or a { text, colour } pair.
export function projectStatus(list) {
  return projectNamed(list, (entry) => {
    if (!(entry instanceof LuaTable)) return null;
    const descript = entry.get("descript");
    if (!(descript instanceof LuaTable)) return null;
    const first = descript.get(1);
    return decodeClientString(first instanceof LuaTable ? first.get(1) : first);
  });
}

// NameTable_VAR maps a random-option id to a printf-style display template
// ("ATQM +%d"); the value is filled in from the item's rolled option at runtime.
export function projectRandomOpt(tbl) {
  return projectNamed(tbl, decodeClientString);
}

// A class is "present on this server" when its party icon ships in the GRF —
// unreleased classes have no icon, which is the same signal /icons/job serves
// from. Only the file table is read; the icon bytes never are.
function jobIconIds(grf) {
  const ids = new Set();
  const re = /\/renewalparty\/icon_jobs_(\d+)\.bmp$/;
  for (const f of grf.files) {
    const m = normalize(f.filename).match(re);
    if (m) ids.add(Number(m[1]));
  }
  return ids;
}

// Jobs are the union of the two id universes: every class pcidentity names, plus
// every class that only shows up as a party icon. `name` is null when the client
// ships no label for that JT (consumers fill those from their own tables).
export function projectJobs(idConsts, labelConsts, iconIds) {
  const jtIds = jtIdsFromConstants(idConsts);
  const labels = jobLabelsFromConstants(labelConsts);
  const byId = new Map();
  for (const [jt, id] of jtIds) {
    if (byId.has(id)) continue;
    byId.set(id, { id, jt, name: labels.get(jt) ?? null, hasIcon: iconIds.has(id) });
  }
  for (const id of iconIds) {
    if (!byId.has(id)) byId.set(id, { id, jt: null, name: null, hasIcon: true });
  }
  return [...byId.values()].sort((a, b) => a.id - b.id);
}

// ---------------------------------------------------------------------------
// Playable classes and hair (--raw classes.json / hair.json)
// ---------------------------------------------------------------------------

// Everything below is a client fact — which sprite/palette files a class is
// drawn from, and what the client itself calls it. What a consumer does with
// that (grouping the classes into a dropdown, renaming them to match a wiki,
// hiding the ones it doesn't want) stays in the consumer, exactly like the
// naming overrides that stay out of items.json/jobs.json.

// The playable classes: JT constant -> [ client job id, clothes-palette basename ].
// This table IS the class universe — one record per key, emitted in this order
// (novice, 1st, 2nd, transcendent, 3rd, 4th, expanded, doram).
//
// Both halves are hardcoded in the client EXE. pcidentity.lub only names a
// little over half of these — it stops before the 4th classes and misses the
// whole expanded branch — and the palette basenames are nowhere in the lua at
// all (jobname.lub covers only NPCs and mobs), so the pairing is maintained
// here. Ids are still read from pcidentity when it has them; the value below is
// the fallback and the cross-check (a disagreement is reported).
//
// Palette basenames were verified against the LATAM data.grf palette listing,
// quirks and all: Crusader's palettes are "크루" (not 크루세이더) and Elemental
// Master's files really are misspelled "elemetal_master". Gender-locked classes
// simply ship no files for the other gender, which the per-gender lookup
// reflects naturally. (The gateway renderer's job_pal_names.txt is the same data
// indexed by job id, extended to mounts/baby/madogear; it differs for the one
// class whose palettes exist under two names — Dancer is 댄서 there, female-only,
// where the character creator's set is 무희, which ships both genders.)
const CLASS_TABLE = {
  JT_NOVICE: [0, "초보자"],

  JT_SWORDMAN: [1, "검사"],
  JT_MAGICIAN: [2, "마법사"],
  JT_ARCHER: [3, "궁수"],
  JT_ACOLYTE: [4, "성직자"],
  JT_MERCHANT: [5, "상인"],
  JT_THIEF: [6, "도둑"],

  JT_KNIGHT: [7, "기사"],
  JT_CRUSADER: [14, "크루"],
  JT_PRIEST: [8, "프리스트"],
  JT_MONK: [15, "몽크"],
  JT_WIZARD: [9, "위저드"],
  JT_SAGE: [16, "세이지"],
  JT_HUNTER: [11, "헌터"],
  JT_BARD: [19, "바드"],
  JT_DANCER: [20, "무희"],
  JT_BLACKSMITH: [10, "제철공"],
  JT_ALCHEMIST: [18, "연금술사"],
  JT_ASSASSIN: [12, "어세신"],
  JT_ROGUE: [17, "로그"],

  JT_NOVICE_H: [4001, "초보자"],
  JT_SWORDMAN_H: [4002, "검사"],
  JT_MAGICIAN_H: [4003, "마법사"],
  JT_ARCHER_H: [4004, "궁수"],
  JT_ACOLYTE_H: [4005, "성직자"],
  JT_MERCHANT_H: [4006, "상인"],
  JT_THIEF_H: [4007, "도둑"],
  JT_KNIGHT_H: [4008, "로드나이트"],
  JT_CRUSADER_H: [4015, "팔라딘"],
  JT_PRIEST_H: [4009, "하이프리스트"],
  JT_MONK_H: [4016, "챔피온"],
  JT_WIZARD_H: [4010, "하이위저드"],
  JT_SAGE_H: [4017, "프로페서"],
  JT_HUNTER_H: [4012, "스나이퍼"],
  JT_BARD_H: [4020, "크라운"],
  JT_DANCER_H: [4021, "집시"],
  JT_BLACKSMITH_H: [4011, "화이트스미스"],
  JT_ALCHEMIST_H: [4019, "크리에이터"],
  JT_ASSASSIN_H: [4013, "어세신크로스"],
  JT_ROGUE_H: [4018, "스토커"],

  JT_RUNE_KNIGHT: [4054, "룬나이트"],
  JT_ROYAL_GUARD: [4066, "로얄가드"],
  JT_ARCH_BISHOP: [4057, "아크비숍"],
  JT_SURA: [4070, "슈라"],
  JT_WARLOCK: [4055, "워록"],
  JT_SORCERER: [4067, "소서러"],
  JT_RANGER: [4056, "레인저"],
  JT_MINSTREL: [4068, "민스트럴"],
  JT_WANDERER: [4069, "원더러"],
  JT_MECHANIC: [4058, "미케닉"],
  JT_GENETIC: [4071, "제네릭"],
  JT_GUILLOTINE_CROSS: [4059, "길로틴크로스"],
  JT_SHADOW_CHASER: [4072, "쉐도우체이서"],

  JT_DRAGON_KNIGHT: [4252, "dragon_knight"],
  JT_IMPERIAL_GUARD: [4258, "imperial_guard"],
  JT_CARDINAL: [4256, "cardinal"],
  JT_INQUISITOR: [4262, "inquisitor"],
  JT_ARCH_MAGE: [4255, "arch_mage"],
  JT_ELEMENTAL_MASTER: [4261, "elemetal_master"],
  JT_WINDHAWK: [4257, "windhawk"],
  JT_TROUBADOUR: [4263, "troubadour"],
  JT_TROUVERE: [4264, "trouvere"],
  JT_MEISTER: [4253, "meister"],
  JT_BIOLO: [4259, "biolo"],
  JT_SHADOW_CROSS: [4254, "shadow_cross"],
  JT_ABYSS_CHASER: [4260, "abyss_chaser"],
  JT_HYPER_NOVICE: [4314, "hyper_novice"],

  JT_SUPERNOVICE: [23, "슈퍼노비스"],
  JT_GUNSLINGER: [24, "건너"],
  JT_REBELLION: [4215, "리벨리온"],
  JT_NINJA: [25, "닌자"],
  JT_KAGEROU: [4211, "kagerou"],
  JT_OBORO: [4212, "oboro"],
  JT_SHINKIRO: [4311, "shinkiro"],
  JT_SHIRANUI: [4312, "shiranui"],
  JT_TAEKWON: [4046, "태권소년"],
  JT_STAR_GLADIATOR: [4047, "권성"],
  JT_STAR_EMPEROR: [4239, "성제"],
  JT_SKY_EMPEROR: [4309, "sky_emperor"],
  JT_SOUL_LINKER: [4049, "소울링커"],
  JT_SOUL_REAPER: [4240, "소울리퍼"],
  JT_SOUL_ASCETIC: [4310, "soul_ascetic"],
  JT_NIGHT_WATCH: [4313, "night_watch"],

  JT_SUMMONER: [4218, "묘족"],
  JT_SPIRIT_HANDLER: [4315, "spirit_handler"],
};

// Body SPRITE basename per class, for the classes where it differs from the
// palette basename above (data/sprite/<race>/몸통/<남|여>/<name>_<남|여>.spr).
// Gravity spells a handful of jobs differently in the two folders — Royal Guard
// is 가드 as a sprite but 로얄가드 as a palette, Ranger is 레인져/레인저 (ㅕ vs ㅓ),
// Crusader the reverse of the palette's abbreviation — so only the exceptions
// are listed; everything else reuses the palette basename. Used to find each class's
// alternative-outfit ("costume_N") sprites; a wrong entry surfaces as a warning
// when the name resolves to no sprite in the GRF.
const CLASS_SPR_NAMES = {
  JT_CRUSADER: "크루세이더", JT_DANCER: "무희", JT_PRIEST_H: "하이프리",
  JT_BARD_H: "클라운", JT_ASSASSIN_H: "어쌔신크로스", JT_ROYAL_GUARD: "가드",
  JT_RANGER: "레인져", JT_REBELLION: "rebellion", JT_SUMMONER: "summoner",
};

// The two doram classes: their sprites and palettes live under the 도람족 tree
// instead of 인간족/몸, so the palette lookup has to know which tree to read.
const DORAM_CLASSES = new Set(["JT_SUMMONER", "JT_SPIRIT_HANDLER"]);

// ragassets indexes the newest expanded 4th classes in its OWN id space (the
// renderer's resolver tables), offset from the client's kRO job ids: the
// STANDING sprite sits at 4302-4308 and the *_RIDING (always-mounted) sprite at
// the client's own 4309-4315. `renderId` is what /render must be asked for, so
// it is the standing id; `id` stays the client's. Everything else renders at its
// client id, where the two are equal.
const RENDER_ID = {
  JT_SKY_EMPEROR: 4302,
  JT_SOUL_ASCETIC: 4303,
  JT_SHINKIRO: 4304,
  JT_SHIRANUI: 4305,
  JT_NIGHT_WATCH: 4306,
  JT_HYPER_NOVICE: 4307,
  JT_SPIRIT_HANDLER: 4308,
};

// (An override list used to sit here, forcing the expanded-branch 4th jobs to be
// reported as released. It was keyed off the client id — 4309+, which ships no
// party icon — and became dead once `unreleased` started asking about the
// renderId instead: LATAM does ship icon_jobs_4302..4308. Don't reintroduce it
// without checking the icon ids first.)

// A few JT constants are spelled differently in the client's two name tables
// (msgstringtable drops the underscore, or keeps the legacy constant), so the
// lookups need the other spelling to resolve.
const CLASS_NAME_ALIAS = {
  JT_ARCH_BISHOP: "ARCHBISHOP",
  JT_SUMMONER: "DO_SUMMONER",
  JT_STAR_GLADIATOR: "STAR",
  JT_SOUL_LINKER: "LINKER",
};

// data/msgstringtable_ml.csv is the multi-language string table the client
// renders from. Each row is comma-separated base64 fields:
// [key, ko, en, …, ptBR(7), …]. Index MSI_JOB_<SUFFIX> by <SUFFIX>, taking the
// pt-BR column and falling back to English when a row is untranslated. It is the
// most current of the client's job-name sources (pcjobnamegender.lub predates
// several renames) but it omits most of the 4th classes, hence the pairing in
// classDisplayName.
export function parseJobMsgNames(text) {
  const out = new Map();
  const b64 = (s) => {
    try {
      return Buffer.from(s || "", "base64").toString("utf-8");
    } catch {
      return "";
    }
  };
  for (const line of text.split(/\r?\n/)) {
    if (!line) continue;
    const cols = line.split(","); // base64 has no commas, so this is safe
    const m = b64(cols[0]).match(/^MSI_JOB_(.+)$/);
    if (!m) continue;
    const pt = b64(cols[7]).trim();
    const en = b64(cols[2]).trim();
    if (pt || en) out.set(m[1], pt || en);
  }
  return out;
}

// The client's own label for a class: msgstringtable first, pcjobnamegender.lub
// second (the only source for the deeper 4th classes), null when the client
// ships neither. Trans classes reuse the base class's label — the lua table has
// no _H rows. Consumers that want prettier or newer names layer their own table
// on top; nothing is invented here.
export function classDisplayName(jt, msgNames, jtLabels) {
  const suffix = CLASS_NAME_ALIAS[jt] ?? jt.replace(/^JT_/, "");
  const lua = (key) => {
    if (jtLabels.has(key)) return decodeClientString(jtLabels.get(key));
    if (key.endsWith("_H") && jtLabels.has(key.slice(0, -2))) {
      return decodeClientString(jtLabels.get(key.slice(0, -2)));
    }
    return null;
  };
  return msgNames.get(suffix) ?? lua(`JT_${suffix}`) ?? lua(jt) ?? null;
}

// One pass over the GRF file table indexing everything the class and hair
// projections need. Palette records keep the GRF entries themselves so the
// swatch sampler can extract the bytes for exactly the palettes it samples.
// Korean path segments: 몸 body, 머리 hair, 몸통 body sprites, 머리통 head
// sprites, 인간족 human race, 도람족 doram race, 남/여 male/female.
function scanPlayerAssets(grf) {
  const bodyPal = new Map(); //     "<palette>|<m|f>"            -> { max, entries: Map<idx, entry> }
  const hairPal = new Map(); //     "<style>|<m|f>"              -> same
  const doramBodyPal = new Map(); //                                same, 도람족 tree
  const doramHairPal = new Map();
  const altBodyPal = new Map(); //  "<palette>|<m|f>|<outfit>"   -> same
  const humanHair = new Map(); //   "m"|"f"                      -> Set<style number>
  const doramHair = new Map();
  const body = new Map(); //        "<sprite>|<m|f>|<race>"      -> the base .spr entry
  const altBody = new Map(); //     "<sprite>|<m|f>|<race>|<n>"  -> { spr, act } entries

  const record = (map, key, idx, f) => {
    let rec = map.get(key);
    if (!rec) map.set(key, (rec = { max: -1, entries: new Map() }));
    rec.max = Math.max(rec.max, idx);
    const prev = rec.entries.get(idx);
    // Same tie-break as findBestEntry: the biggest copy is the complete one.
    if (!prev || f.uncompSize > prev.uncompSize) rec.entries.set(idx, f);
  };
  const g2 = (g) => (g === "남" ? "m" : "f");

  for (const f of grf.files) {
    if (!(f.flags & 0x01)) continue;
    const n = normalize(f.filename);

    if (n.startsWith("data/palette/")) {
      const rel = n.slice("data/palette/".length);
      let m = rel.match(/^몸\/([^/]+)_(남|여)_(\d+)\.pal$/);
      if (m) { record(bodyPal, `${m[1]}|${g2(m[2])}`, +m[3], f); continue; }
      m = rel.match(/^머리\/머리(\d+)_(남|여)_(\d+)\.pal$/);
      if (m) { record(hairPal, `${m[1]}|${g2(m[2])}`, +m[3], f); continue; }
      m = rel.match(/^도람족\/(?:body|몸)\/([^/]+)_(남|여)_(\d+)\.pal$/);
      if (m) { record(doramBodyPal, `${m[1]}|${g2(m[2])}`, +m[3], f); continue; }
      m = rel.match(/^도람족\/(?:hair|머리)\/(?:머리)?(\d+)_(남|여)_(\d+)\.pal$/);
      if (m) { record(doramHairPal, `${m[1]}|${g2(m[2])}`, +m[3], f); continue; }
      // Alternative-outfit clothes palettes, a parallel set per outfit:
      // 몸/costume_1/<palette>_<남|여>_<idx>_1.pal (doram under 도람족/body/).
      m = rel.match(/^(?:몸|도람족\/body)\/costume_(\d+)\/(.+)_(남|여)_(\d+)_(\d+)\.pal$/);
      if (m && m[1] === m[5]) record(altBodyPal, `${m[2]}|${g2(m[3])}|${m[1]}`, +m[4], f);
      continue;
    }

    // Body sprites, base and alternative-outfit. The renderer needs both the
    // .act and the .spr before it will draw an outfit, so each extension is
    // recorded separately and altOutfits requires the pair.
    {
      const m = n.match(
        /^data\/sprite\/(인간족|도람족)\/몸통\/(남|여)\/(?:costume_(\d+)\/)?(.+?)_(남|여)(?:_(\d+))?\.(spr|act)$/,
      );
      if (m) {
        const race = m[1] === "인간족" ? "human" : "doram";
        const key = `${m[4]}|${g2(m[2])}|${race}`;
        if (!m[3]) {
          if (m[7] === "spr" && !m[6]) body.set(key, f);
        } else if (m[6] === m[3]) {
          if (!altBody.has(`${key}|${m[3]}`)) altBody.set(`${key}|${m[3]}`, {});
          altBody.get(`${key}|${m[3]}`)[m[7]] = f;
        }
        continue;
      }
    }

    if (n.endsWith(".spr")) {
      const m = n.match(/^data\/sprite\/(인간족|도람족)\/머리통\/(남|여)\/(\d+)_(남|여)\.spr$/);
      if (!m) continue;
      const styles = m[1] === "인간족" ? humanHair : doramHair;
      const g = g2(m[2]);
      if (!styles.has(g)) styles.set(g, new Set());
      styles.get(g).add(+m[3]);
    }
  }

  return { bodyPal, hairPal, doramBodyPal, doramHairPal, altBodyPal, humanHair, doramHair, body, altBody };
}

// A .pal is 256 RGBA entries covering the WHOLE sprite (skin, outlines, the
// magenta transparency key…), so a single palette cannot say which entries are
// the dye. The dye region is whatever CHANGES between the numbered palettes of
// one class/style: diff each against a reference, keep the indices that vary and
// average that palette's colors over them (chroma-weighted hue mode, so
// outlines and highlights don't wash the hue out). The result is one "#rrggbb"
// per palette — a swatch a picker can render without downloading the palettes.
// Palettes that are missing or too short come back null, so the array stays
// index-aligned with the palette ids the renderer takes.
export function paletteSwatches(pals) {
  const ok = (p) => p && p.length >= 1024;
  if (pals.filter(ok).length < 2) return pals.map(() => null);
  const refIdx = pals.findIndex(ok);
  const ref = pals[refIdx];

  const isMagenta = (r, g, b) => r > 200 && b > 200 && g < 80;
  // Per-palette dye region: the entries where THIS palette differs from the
  // reference. Computing it per palette (rather than across all of them at once)
  // keeps outliers — an all-black "dark outfit" palette that retints everything
  // — from widening the normal ones' region to the whole sprite.
  const diffRegion = (p) => {
    const out = [];
    for (let i = 1; i < 256; i++) {
      const r0 = ref[i * 4], g0 = ref[i * 4 + 1], b0 = ref[i * 4 + 2];
      const r = p[i * 4], g = p[i * 4 + 1], b = p[i * 4 + 2];
      if (isMagenta(r0, g0, b0) || isMagenta(r, g, b)) continue;
      if (Math.abs(r - r0) + Math.abs(g - g0) + Math.abs(b - b0) > 48) out.push(i);
    }
    return out;
  };
  const regions = pals.map((p, n) => (ok(p) && n !== refIdx ? diffRegion(p) : []));
  // The reference palette (and any palette identical to it) samples the union of
  // everyone else's regions — the original colors of the dyed area.
  const union = [...new Set(regions.flat())].sort((a, b) => a - b);
  if (!union.length) return pals.map(() => null);

  return pals.map((p, n) => {
    if (!ok(p)) return null;
    const region = regions[n].length ? regions[n] : union;
    const colors = region.map((i) => {
      const r = p[i * 4], g = p[i * 4 + 1], b = p[i * 4 + 2];
      const max = Math.max(r, g, b), min = Math.min(r, g, b);
      return { r, g, b, lum: r + g + b, chroma: max - min, hue: hueOf(r, g, b, max, min) };
    });
    // The dye hue is the chroma-weighted mode over 30° hue bins; averaging only
    // that bin keeps the swatch vivid. All-neutral palettes (white/gray dyes)
    // have no colorful bin and fall back to a mid-luminance average.
    const bins = new Map();
    for (const c of colors) {
      if (c.chroma < 25) continue;
      const bin = Math.floor(c.hue / 30);
      bins.set(bin, (bins.get(bin) ?? 0) + c.chroma);
    }
    let band;
    if (bins.size) {
      const top = [...bins.entries()].sort((a, b) => b[1] - a[1])[0][0];
      band = colors.filter((c) => c.chroma >= 25 && Math.floor(c.hue / 30) === top);
      band.sort((a, b) => a.lum - b.lum);
      band = band.slice(Math.floor(band.length * 0.3), Math.max(1, Math.ceil(band.length * 0.85)));
    } else {
      colors.sort((a, b) => a.lum - b.lum);
      band = colors.slice(Math.floor(colors.length * 0.35), Math.max(1, Math.ceil(colors.length * 0.8)));
    }
    const avg = band.reduce((acc, c) => ({ r: acc.r + c.r, g: acc.g + c.g, b: acc.b + c.b }), { r: 0, g: 0, b: 0 });
    const hex = (v) => Math.round(v / band.length).toString(16).padStart(2, "0");
    return `#${hex(avg.r)}${hex(avg.g)}${hex(avg.b)}`;
  });
}

function hueOf(r, g, b, max, min) {
  if (max === min) return 0;
  const d = max - min;
  let h;
  if (max === r) h = ((g - b) / d) % 6;
  else if (max === g) h = (b - r) / d + 2;
  else h = (r - g) / d + 4;
  return (h * 60 + 360) % 360;
}

// { count, swatches } for one palette record — count is the highest palette id
// plus one, so it stays the range /render accepts even when the middle of the
// range is missing (those swatches are null).
function paletteSet(rec, read) {
  if (!rec) return null;
  const count = rec.max + 1;
  const pals = [];
  for (let i = 0; i < count; i++) {
    const entry = rec.entries.get(i);
    pals.push(entry ? read(entry) : null);
  }
  return { count, swatches: paletteSwatches(pals) };
}

// Alternative outfits ("estilo de roupa" / body style): a parallel set of body
// sprites the client ships under 몸통/<gender>/costume_<N>/, which the renderer
// draws instead of the normal body when asked for outfit N. The 3rd classes are
// the well-known ones, but the client has them for a few others too (Novice,
// Cardinal, Inquisitor, Arch Mage, Kagerou/Oboro) — so the set is read from the
// GRF rather than hardcoded. An outfit counts only when both its .act and .spr
// exist (the renderer's own test) AND the .spr differs from the normal body's:
// Gravity ships stub costume_1 folders for a few jobs whose sprite is a
// byte-for-byte copy of the base one, so the "alternative" outfit would render
// exactly the same character. Its clothes-color palettes live in a matching
// palette/몸/costume_<N>/ folder and are a DIFFERENT set from the normal body's
// (sometimes fewer, sometimes none), so each outfit carries its own.
function altOutfits(scan, jt, palette, race, palettes, read) {
  const sprite = (CLASS_SPR_NAMES[jt] ?? palette).toLowerCase();
  const key = (g) => `${sprite}|${g}|${race}`;
  if (!scan.body.has(key("m")) && !scan.body.has(key("f"))) {
    console.error(`  ! ${jt} (${sprite}): no body sprite under that name — outfits not checked`);
    return [];
  }
  const digest = (entry) => createHash("md5").update(read(entry)).digest("hex");
  const out = [];
  for (let n = 1; n <= 8; n++) {
    const outfitPalettes = {};
    // Gender-locked classes ship an outfit sprite for the gender they can't be
    // (Kagerou has a female costume_1 body), so follow the base palettes — the
    // same signal the gender lock itself is read from.
    for (const g of Object.keys(palettes)) {
      const files = scan.altBody.get(`${key(g)}|${n}`);
      const base = scan.body.get(key(g));
      if (!files?.spr || !files.act || !base) continue;
      if (base.uncompSize === files.spr.uncompSize && digest(base) === digest(files.spr)) continue;
      // The palette name can differ from the sprite name (Royal Guard is 가드 as
      // a sprite, 로얄가드 as a palette); no palettes at all means the outfit
      // only ever renders in its own colors.
      const pal = scan.altBodyPal.get(`${palette.toLowerCase()}|${g}|${n}`);
      outfitPalettes[g] = paletteSet(pal, read) ?? { count: 0, swatches: [] };
    }
    if (Object.keys(outfitPalettes).length) out.push({ n, palettes: outfitPalettes });
  }
  return out;
}

// One record per playable class: who it is (client id, JT constant, client
// label), how ragassets draws it (renderId, sprite/palette basenames, race) and
// every clothes-color palette the client ships for it, per gender and per
// alternative outfit. `unreleased` is true when the server has no party icon for
// the class — the same file /icons/job serves from, and the only client-side
// signal that a class exists as data but isn't playable yet.
//
// `read(entry)` extracts a GRF entry's bytes; injected so the projection can be
// exercised without a GRF.
export function projectClasses(scan, { jtIds, jtLabels, msgNames, iconIds }, read) {
  const out = [];
  for (const [jt, [tableId, palette]] of Object.entries(CLASS_TABLE)) {
    // pcidentity spells a few classes without the underscore (JT_ARCHBISHOP),
    // hence the same alias the name lookup uses.
    const clientId = jtIds.get(jt) ?? jtIds.get(`JT_${CLASS_NAME_ALIAS[jt] ?? ""}`);
    if (clientId != null && clientId !== tableId) {
      console.error(`  ! ${jt}: pcidentity says ${clientId}, table says ${tableId} — using the client's`);
    }
    const id = clientId ?? tableId;
    const renderId = RENDER_ID[jt] ?? id;
    const race = DORAM_CLASSES.has(jt) ? "doram" : "human";

    const byName = race === "doram" ? scan.doramBodyPal : scan.bodyPal;
    const palettes = {};
    for (const g of ["m", "f"]) {
      const set = paletteSet(byName.get(`${palette}|${g}`), read);
      if (set) palettes[g] = set;
    }
    if (!Object.keys(palettes).length) console.error(`  ! ${jt} (${palette}): no clothes palettes found`);

    out.push({
      id,
      renderId,
      jt,
      name: classDisplayName(jt, msgNames, jtLabels),
      race,
      sprite: CLASS_SPR_NAMES[jt] ?? palette,
      palette,
      palettes,
      outfits: altOutfits(scan, jt, palette, race, palettes, read),
      unreleased: !iconIds.has(renderId),
    });
  }
  return out;
}

// One record per race+gender. Styles come from the hair sprites
// (data/sprite/<race>/머리통/<g>/<n>_<g>.spr); `colors` is how many recolor
// palettes that style has (data/palette/머리/머리<n>_<g>_<i>.pal, doram under
// 도람족/). Styles above the palette range simply have no recolors (colors: 0) —
// only the sprite's own coloring. The hair dyes are the same hues across styles,
// so one representative `swatches` row is sampled from the richest style rather
// than repeating it for each.
export function projectHair(scan, read) {
  const build = (race, styleSet, palMap, gender) => {
    const styles = [...(styleSet.get(gender) ?? [])]
      .sort((a, b) => a - b)
      .map((n) => ({ n, colors: (palMap.get(`${n}|${gender}`)?.max ?? -1) + 1 }));
    const richest = styles.reduce((a, b) => (b.colors > (a?.colors ?? 0) ? b : a), null);
    const set = richest?.colors ? paletteSet(palMap.get(`${richest.n}|${gender}`), read) : null;
    return { race, gender, styles, swatches: set?.swatches ?? [] };
  };
  return [
    build("human", scan.humanHair, scan.hairPal, "m"),
    build("human", scan.humanHair, scan.hairPal, "f"),
    build("doram", scan.doramHair, scan.doramHairPal, "m"),
    build("doram", scan.doramHair, scan.doramHairPal, "f"),
  ];
}

// Any Cyrillic surviving repairLatamAccents is a row-0xAC cell with no known
// accent behind it, so it ships as a wrong letter to every reader of the table:
// the calculator parses these description lines, and "Nьvel necessрrio" fails a
// "N[ií]vel necess[aá]rio" match as surely as it misreads to a player. Report it
// rather than throwing — one mangled word is not worth blocking a whole publish,
// but it must never pass unremarked.
export function reportUnrepairedCyrillic(name, records) {
  const chars = new Map();
  const walk = (v) => {
    if (typeof v === "string") {
      for (const ch of v) if (CYRILLIC.test(ch)) chars.set(ch, (chars.get(ch) ?? 0) + 1);
    } else if (Array.isArray(v)) v.forEach(walk);
    else if (v && typeof v === "object") Object.values(v).forEach(walk);
  };
  walk(records);
  if (!chars.size) return;
  const list = [...chars]
    .map(([ch, n]) => `${ch} (U+${ch.codePointAt(0).toString(16).toUpperCase().padStart(4, "0")}) x${n}`)
    .join(", ");
  console.error(`  ! ${name}: Cyrillic left unrepaired — ${list}`);
  console.error("    Work out which accent each stands for and add it to LATAM_ACCENTS.");
}

function extractRawTables(grfPath, outDir, args) {
  const dest = resolve(outDir);
  mkdirSync(dest, { recursive: true });

  const lubPath = resolveItemInfoPath(args);
  if (!lubPath) {
    console.error("iteminfo_new.lub not found next to the GRF (System/) — pass --iteminfo <path>");
    process.exit(1);
  }

  const grf = openGrf(grfPath);

  // Tables are collected in memory and only written once every one of them has
  // been built. A half-regenerated resources/raw is worse than a stale one: the
  // deploy skill tars this directory as-is onto a live /raw, with no PR in
  // between to catch it. Throwing (rather than exiting) also lets the `finally`
  // below close the GRF.
  const tables = new Map();
  const write = (name, records) => {
    if (!records.length) throw new Error(`${name}: no records — refusing to write an empty table`);
    reportUnrepairedCyrillic(name, records);
    tables.set(name, records);
  };

  try {
    const lubs = collectGrfFiles(grf, Object.values(RAW_LUB_PATHS).map(normalize));
    const lub = (key) => lubs.get(normalize(RAW_LUB_PATHS[key]));

    // Items — iteminfo_new.lub lives next to the GRF, not inside it; the aegis
    // names come from a plain-text table that does.
    const moveInfo = findBestEntry(grf, normalize(RAW_ITEMMOVE_PATH));
    const aegisMap = moveInfo
      ? parseAegisMap(Buffer.from(extractFile(grf, moveInfo)).toString("latin1"))
      : new Map();
    console.error(`items from ${lubPath} (${aegisMap.size} aegis names)`);
    // Box contents ride along on the item row that opens the box, so they are
    // read before the projection rather than written as a table of their own.
    if (!lub("packageitem")) {
      throw new Error(`${RAW_LUB_PATHS.packageitem} not found in the GRF`);
    }
    const packages = projectPackages(runChunk(lub("packageitem")).get("tbl"));
    const items = projectItems(runChunk(readFileSync(lubPath)).get("tbl"), aegisMap, buildViewResolver(grf), packages);
    write("items.json", items);
    const recovered = items.filter((i) => !i.view && i.spriteView).length;
    console.error(`  ${recovered} views recovered from resource names (ClassNum 0)`);
    // A box whose id has no iteminfo row has nowhere to hang its drop list, so
    // it is dropped — say how many, because the alternative reads as "the client
    // stopped shipping contents for them".
    const boxed = items.filter((i) => i.contains).length;
    console.error(
      `  ${boxed}/${packages.size} boxes carry contents (${packages.size - boxed} reference ids iteminfo has no row for)`,
    );
    // Losing the whole table (a re-keyed chunk, a locale swap) has to fail loudly:
    // items.json would still look perfectly valid, just with every box empty.
    if (boxed < packages.size * 0.5) {
      throw new Error(`items.json: only ${boxed}/${packages.size} boxes matched an item — check ${RAW_LUB_PATHS.packageitem}`);
    }

    // Jobs — read positionally out of the constant pools, see parseLuaConstants.
    // classes.json below pairs the same two pools against the palette scan.
    const idConsts = lub("pcidentity") ? parseLuaConstants(lub("pcidentity")) : [];
    const labelConsts = lub("pcjobnamegender") ? parseLuaConstants(lub("pcjobnamegender")) : [];
    const iconIds = jobIconIds(grf);
    write("jobs.json", projectJobs(idConsts, labelConsts, iconIds));

    // Each of the three name tables is keyed by consts a companion chunk defines,
    // so the pair runs over one shared globals table, seed chunk first.
    const globalsOf = (...keys) => {
      const g = new LuaTable();
      for (const key of keys) {
        if (!lub(key)) {
          console.error(`! ${RAW_LUB_PATHS[key]} not found in the GRF`);
          process.exit(1);
        }
        runChunkInto(lub(key), g);
      }
      return g;
    };

    // skillid.lub defines SKID, which skillinfolist and skilldescript both key
    // themselves by; the names come from the former, the tooltips the latter.
    // They run into separate globals so that namedTable's "biggest table"
    // fallback can never mistake SKILL_DESCRIPT for the skill name table.
    const skillGlobals = globalsOf("skillid", "skillinfolist");
    const skillNames = namedTable(skillGlobals, "SkillInfoList_string", "SKID");
    const clientSkid = skidMap(skillGlobals.get("SKID"));
    if (!clientSkid.size) throw new Error(`${RAW_LUB_PATHS.skillid}: no SKID table`);
    const skid = withSkidAdditions(clientSkid);
    const clientSkidIds = new Set(clientSkid.values());
    for (const [konst, id] of Object.entries(SKID_ADDITIONS)) {
      if (clientSkid.has(konst) || clientSkidIds.has(id)) {
        console.error(`  skillid.lub now defines ${konst} or id ${id} — delete it from SKID_ADDITIONS`);
      }
    }
    const skillDescript = globalsOf("skillid", "skilldescript").get("SKILL_DESCRIPT");
    if (!(skillDescript instanceof LuaTable)) {
      throw new Error(`${RAW_LUB_PATHS.skilldescript}: no SKILL_DESCRIPT table`);
    }
    // The cast/delay table is keyed by the same SKID consts again, and again
    // runs into its own globals so no fallback can confuse the three.
    const skillDelayList = globalsOf("skillid", "skilldelay").get("SKILL_DELAY_LIST");
    if (!(skillDelayList instanceof LuaTable)) {
      throw new Error(`${RAW_LUB_PATHS.skilldelay}: no SKILL_DELAY_LIST table`);
    }
    // Max levels come from the chunk that *builds* SkillInfoList_data, not from
    // the SKILL_INFO_LIST it then merges the names into: that merge is a
    // `pairs()` loop this VM deliberately doesn't run, so the global it targets
    // is left empty (see OP.TFORLOOP) and only looks like the table to read.
    const skillInfo = globalsOf("skillid", "skillinfodata").get("SkillInfoList_data");
    if (!(skillInfo instanceof LuaTable)) {
      throw new Error(`${RAW_LUB_PATHS.skillinfodata}: no SkillInfoList_data table`);
    }
    const skills = projectSkills(skillNames, skillDescript, skillDelayList, skillInfo, skid);
    // The checks below hold the client's own rows to account, not the ones this
    // file adds: an id the client leaves unnamed rarely has a tooltip or an info
    // row either, and counting those would lower every bar they set.
    const clientNamed = new Map(projectSkills(skillNames).map((s) => [s.id, s.name]));
    const own = skills.filter((s) => clientNamed.has(s.id));
    const described = own.filter((s) => s.description).length;
    const timed = own.filter((s) => s.delay).length;
    const levelled = own.filter((s) => s.maxLevel !== null).length;
    console.error(
      `skills: ${described}/${own.length} carry a description, ${timed} carry cast/delay times, ${levelled} a max level`,
    );
    // Say what the curated names did, so a patch that starts naming one of them
    // (or drops a constant) shows up in the cycle's log instead of going unseen.
    const { report } = resolveUnnamedSkills(skid, clientNamed);
    console.error(
      `  ${skills.length - own.length} more named by UNNAMED_SKILL_NAMES or their parent; ${skills.filter((s) => "parent" in s).length} follow-up hits carry a parent`,
    );
    if (report.shadowed.length) {
      console.error(`  the client now names ${report.shadowed.join(", ")} — delete them from UNNAMED_SKILL_NAMES`);
    }
    if (report.unknown.length) console.error(`  ! unresolved skill-name entries: ${report.unknown.join(", ")}`);
    if (report.unnamed.length) {
      console.error(`  ${report.unnamed.length} skill ids still have no name: ${report.unnamed.join(", ")}`);
    }
    // Every named skill has a max level in the current client, so anything short
    // of all of them means the two tables have drifted apart, not that the
    // client stopped shipping one.
    if (levelled < own.length) {
      console.error(`  ! ${own.length - levelled} skills have no MaxLv in ${RAW_LUB_PATHS.skillinfodata}`);
    }
    // A wholesale miss means the tooltips came from the wrong chunk (or a
    // re-keyed one), which reads downstream as "the client dropped them".
    if (described < own.length * 0.5) {
      throw new Error(`skills.json: only ${described}/${own.length} descriptions — check ${RAW_LUB_PATHS.skilldescript}`);
    }
    // Same trap for the timings, at a lower bar: only ~60% of the named skills
    // have a delay row at all (passives never do), so the failure to catch is
    // the table going empty, not it being partial.
    if (timed < own.length * 0.25) {
      throw new Error(`skills.json: only ${timed}/${own.length} with cast/delay times — check ${RAW_LUB_PATHS.skilldelay}`);
    }
    write("skills.json", skills);

    // enumvar defines the consts the random-option name table may key by.
    write("randomopt.json", projectRandomOpt(globalsOf("enumvar", "randomopt").get("NameTable_VAR")));

    // efstids defines the EFST_* consts StateIconList keys by.
    const statusGlobals = globalsOf("efstids", "stateiconinfo");
    write("status.json", projectStatus(namedTable(statusGlobals, "StateIconList")));

    // Classes and hair — one pass over the file table indexes every player
    // palette and body/hair sprite, then the projections sample the swatches.
    const scan = scanPlayerAssets(grf);
    const read = (entry) => extractFile(grf, entry);
    const msgEntry = findBestEntry(grf, normalize(RAW_MSGSTRING_PATH));
    if (!msgEntry) console.error("  ! msgstringtable_ml.csv missing — class names fall back to lua");
    write(
      "classes.json",
      projectClasses(
        scan,
        {
          jtIds: jtIdsFromConstants(idConsts),
          jtLabels: jobLabelsFromConstants(labelConsts),
          msgNames: msgEntry ? parseJobMsgNames(Buffer.from(read(msgEntry)).toString("latin1")) : new Map(),
          iconIds,
        },
        read,
      ),
    );
    write("hair.json", projectHair(scan, read));

  } finally {
    closeGrf(grf);
  }

  // Warn when a table collapses — the realistic failure after a client update is
  // a .lub moving and a table losing most of its rows, not losing all of them.
  const summary = [];
  for (const [name, records] of tables) {
    const path = join(dest, name);
    const before = existsSync(path) ? JSON.parse(readFileSync(path, "utf8")).length : null;
    writeFileSync(path, JSON.stringify(records));
    const delta = before === null ? "new" : `was ${before}`;
    summary.push(`  ${name} — ${records.length} (${delta})`);
    if (before !== null && records.length < before * 0.9) {
      console.error(`! ${name} lost ${before - records.length} of ${before} rows — check the client tables before deploying`);
    }
  }
  console.error(`\nraw tables → ${dest}\n${summary.join("\n")}`);
}

// Take the named global, falling back to the biggest table the chunk defined
// (skipping the const table that seeded it). The fallback is a safety net, not
// the plan: "biggest table" would silently republish the wrong data under a name
// every consumer trusts if a client update added a larger one, so a miss is loud.
function namedTable(globals, name, skipKey) {
  const byName = globals.get(name);
  if (byName instanceof LuaTable) return byName;

  let best = null;
  for (const [k, v] of globals.map) {
    if (k === skipKey) continue;
    if (v instanceof LuaTable && (!best || v.map.size > best.map.size)) best = v;
  }
  console.error(`! ${name} not defined by the chunk — falling back to its largest table (${best?.map.size ?? 0} rows)`);
  return best;
}

// Run the CLI only when executed directly (not when imported by a test, or from
// a context with no script path at all such as `node -e`).
if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) main();
