/**
 * GRF header parsing, versions 0x200 and 0x300.
 *
 * 0x300 differs from 0x200 in three places, all of which the validator used to ignore: the table offset
 * is a 64-bit value spanning bytes 30..37 (so what 0x200 calls the seed is that offset's high half), the
 * file count is literal rather than nFiles - seed - 7, and an extra Int32 sits between the header and the
 * file table. A valid 0x300 archive therefore failed with "Failed to read/parse compacted file table",
 * and since startup validation is fatal the server refused to boot.
 *
 * Both halves matter here: the synthetic headers pin the field arithmetic, and the real fixtures prove
 * an actual archive of each version parses end to end. See tests/fixtures/README.md for their provenance.
 */
const test = require('node:test');
const assert = require('node:assert');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');

const StartupValidator = require('../src/validators/startupValidator');

const FIXTURES = path.join(__dirname, 'fixtures');

/** Build a 46-byte GRF header. `tableOffset` is written as u64 when `big` is set (0x300 layout). */
function buildHeader({ tableOffset, seed = 0, nFiles, version, big = false }) {
  const b = Buffer.alloc(46);
  b.write('Master of Magic\0', 'ascii');
  if (big) {
    b.writeBigUInt64LE(BigInt(tableOffset), 30);
  } else {
    b.writeUInt32LE(tableOffset, 30);
    b.writeUInt32LE(seed, 34);
  }
  b.writeUInt32LE(nFiles, 38);
  b.writeUInt32LE(version, 42);
  return b;
}

/** Run the validator's private header reader against a buffer written to a temp file. */
function readHeader(buffer) {
  const file = path.join(fs.mkdtempSync(path.join(os.tmpdir(), 'grfhdr-')), 'h.grf');
  fs.writeFileSync(file, buffer);
  const fd = fs.openSync(file, 'r');
  try {
    return new StartupValidator()._readGrfHeader46(fd);
  } finally {
    fs.closeSync(fd);
    fs.rmSync(path.dirname(file), { recursive: true, force: true });
  }
}

test('0x200: table offset is 32-bit and the file count is nFiles - seed - 7', () => {
  const h = readHeader(buildHeader({ tableOffset: 1000, seed: 5, nFiles: 100, version: 0x200 }));
  assert.strictEqual(h.ok, true);
  assert.strictEqual(h.tableOffset, 1000);
  assert.strictEqual(h.fileCount, 88);
});

test('0x300: table offset is 64-bit and the file count is literal', () => {
  const h = readHeader(buildHeader({ tableOffset: 2000, nFiles: 100, version: 0x300, big: true }));
  assert.strictEqual(h.ok, true);
  assert.strictEqual(h.tableOffset, 2000);
  // Reading this with the 0x200 layout gives 93 — nFiles minus a "seed" that is really the
  // offset's high half, minus 7.
  assert.strictEqual(h.fileCount, 100);
});

test('0x300: a table offset above 4 GB survives instead of being truncated', () => {
  const h = readHeader(buildHeader({ tableOffset: 5_000_000_000, nFiles: 7, version: 0x300, big: true }));
  assert.strictEqual(h.tableOffset, 5_000_000_000, 'a 32-bit read would yield 705032704');
});

test('a header shorter than 46 bytes is rejected', () => {
  const h = readHeader(Buffer.alloc(20));
  assert.strictEqual(h.ok, false);
});

test('a wrong signature is rejected', () => {
  const b = buildHeader({ tableOffset: 0, nFiles: 7, version: 0x200 });
  b.write('Not a GRF file\0', 'ascii');
  assert.strictEqual(readHeader(b).ok, false);
});

test('byte 15 must be NUL, or the signature comparison fails', () => {
  const b = buildHeader({ tableOffset: 0, nFiles: 7, version: 0x200 });
  b[15] = 0x41;
  assert.strictEqual(readHeader(b).ok, false);
});

// Real archives, end to end through validateGrfFormat() and the loader.
const ARCHIVES = [
  ['with-files.grf', true, '0x200'],
  ['with-files-v300.grf', true, '0x300'],
  ['incorrect-version.grf', false, 'version 0x103'],
  ['not-grf.grf', false, 'plain text'],
  ['corrupted.grf', false, 'empty file'],
];

for (const [name, shouldBeValid, what] of ARCHIVES) {
  test(`${name} (${what}) is ${shouldBeValid ? 'accepted' : 'rejected'}`, async () => {
    const result = await new StartupValidator().validateGrfFormat(path.join(FIXTURES, name));
    assert.strictEqual(
      result.valid,
      shouldBeValid,
      `${name}: ${result.reason || '(no reason given)'}`
    );
  });
}
