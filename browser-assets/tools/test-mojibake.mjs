#!/usr/bin/env node
/**
 * test-mojibake.mjs
 *
 * Testa as funções de detecção e correção de mojibake.
 *
 * Uso:
 *  node tools/test-mojibake.mjs
 */

// Load the CommonJS build on purpose.
//
// The package's ESM build resolves iconv-lite through a bundler shim that needs a global `require`.
// Under `node -e` one exists and everything appears to work; inside a real .mjs there is none, the
// shim throws, iconv-lite is null, and isMojibake/fixMojibake/toMojibake silently become identity
// functions. That made `npm run convert:encoding` produce an empty mapping while reporting success,
// and made the validators flag encoding errors in archives the server reads perfectly.
import { createRequire } from "module";
const { isMojibake, fixMojibake, toMojibake, normalizeFilename, normalizeEncodingPath, hasIconvLite } = createRequire(import.meta.url)("@chicowall/grf-loader");

console.log("=".repeat(70));
console.log("Mojibake Detection & Fixing Test");
console.log("=".repeat(70));
console.log("");

console.log(`iconv-lite available: ${hasIconvLite()}`);
console.log("");

// Test cases
const testCases = [
  { korean: "유저인터페이스", description: "User Interface" },
  { korean: "아이템", description: "Item" },
  { korean: "스프라이트", description: "Sprite" },
  { korean: "몬스터", description: "Monster" },
  { korean: "데이터", description: "Data" },
  { korean: "망토", description: "Mantle/Cape" },
  { korean: "카드", description: "Card" },
];

console.log("1. Korean → Mojibake → Fixed");
console.log("-".repeat(70));

for (const { korean, description } of testCases) {
  const mojibake = toMojibake(korean);
  const fixed = fixMojibake(mojibake);
  const detected = isMojibake(mojibake);
  const match = fixed === korean ? "✅" : "❌";

  console.log(`${description}:`);
  console.log(`  Korean:   ${korean}`);
  console.log(`  Mojibake: ${mojibake}`);
  console.log(`  Detected: ${detected}`);
  console.log(`  Fixed:    ${fixed} ${match}`);
  console.log("");
}

console.log("2. Path Normalization");
console.log("-".repeat(70));

const testPaths = [
  "data\\texture\\À¯ÀúÀÎÅÍÆäÀÌ½º\\cardbmp\\test.bmp",
  "data\\sprite\\¾ÆÀÌÅÛ\\monster.spr",
  "data\\texture\\normal\\test.bmp",  // Should not change
];

for (const path of testPaths) {
  const normalized = normalizeEncodingPath(path);
  const changed = path !== normalized;
  console.log(`Original:   ${path}`);
  console.log(`Normalized: ${normalized} ${changed ? "✅ (fixed)" : "(unchanged)"}`);
  console.log("");
}

console.log("3. Detection Tests");
console.log("-".repeat(70));

const detectionTests = [
  { str: "À¯ÀúÀÎÅÍÆäÀÌ½º", expected: true, desc: "Mojibake Korean" },
  { str: "유저인터페이스", expected: false, desc: "Proper Korean" },
  { str: "normal_filename.txt", expected: false, desc: "ASCII filename" },
  { str: "test_ÀÌ¹ÌÁö.bmp", expected: true, desc: "Mixed mojibake" },
  { str: "données.txt", expected: false, desc: "French accents" },
];

for (const { str, expected, desc } of detectionTests) {
  const detected = isMojibake(str);
  const match = detected === expected ? "✅" : "❌";
  console.log(`${desc}: "${str}"`);
  console.log(`  Expected: ${expected}, Got: ${detected} ${match}`);
  console.log("");
}

console.log("=".repeat(70));
console.log("Done!");
