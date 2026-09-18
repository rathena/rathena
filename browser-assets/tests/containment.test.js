/**
 * Path containment in Client.getFile().
 *
 * getFile() is the funnel for both network-reachable file routes -- POST /batch and the GET /*
 * wildcard -- so every path the client can name arrives here. It used to build its local path with
 * path.join(__dirname, '..', '..', filePath) and read it with no normalisation and no containment
 * check, which made the repository root a document root: .env, .git/config and the server source
 * were all downloadable, no "../" required.
 *
 * These tests need no GRF. Requests that reach the archive lookup simply return null when no
 * archive is loaded, which is the same "nothing served" outcome the assertions check for.
 */
const test = require('node:test');
const assert = require('node:assert');
const fs = require('node:fs');
const path = require('node:path');

const Client = require('../src/controllers/clientController');

const ROOT = path.resolve(__dirname, '..');
const HAPPY_REL = 'data/containment-fixture.txt';
const HAPPY_ABS = path.join(ROOT, 'data', 'containment-fixture.txt');
const HAPPY_BODY = 'served-from-an-asset-directory';

test.before(() => {
  fs.mkdirSync(path.dirname(HAPPY_ABS), { recursive: true });
  fs.writeFileSync(HAPPY_ABS, HAPPY_BODY);
});

test.after(() => {
  fs.rmSync(HAPPY_ABS, { force: true });
});

// Every one of these resolved to a real, readable file before containment was added.
// The four marked (leaked) are the ones verified to have been served by the pre-fix code.
const MUST_NOT_SERVE = [
  ['.env', 'environment file at the repository root (leaked)'],
  ['.git/config', 'VCS metadata (leaked)'],
  ['package.json', 'project manifest (leaked)'],
  ['index.js', 'server source (leaked)'],
  ['src/controllers/clientController.js', 'server source, nested'],
  ['logs/missing-files.log', 'operational log'],
  ['resources/DATA.INI', 'client configuration'],
  ['../../../../../../Windows/win.ini', 'classic traversal'],
  ['data/../.env', 'traversal that re-enters via an allowed directory'],
  ['data\\..\\.env', 'same, with Windows separators'],
  ['DATA/../.env', 'same, exercising case-insensitive roots'],
  ['C:/Windows/win.ini', 'absolute path, Windows'],
  ['/etc/passwd', 'absolute path, POSIX'],
  ['data/../../ragnarock/roBrowserLegacy/README.md', 'escape into a sibling checkout'],
];

for (const [candidate, why] of MUST_NOT_SERVE) {
  test(`refuses ${JSON.stringify(candidate)} — ${why}`, async () => {
    const content = await Client.getFile(candidate);
    assert.ok(
      !content || content.length === 0,
      `getFile() returned ${content && content.length} bytes for ${candidate}`
    );
  });
}

test('a NUL byte in the path is refused rather than truncating it', async () => {
  const content = await Client.getFile('data/containment-fixture.txt\u0000.png');
  assert.ok(!content || content.length === 0);
});

test('still serves a legitimate file from an asset directory', async () => {
  const content = await Client.getFile(HAPPY_REL);
  assert.ok(content, 'the happy path must keep working — containment is not a blanket deny');
  assert.strictEqual(content.toString().trim(), HAPPY_BODY);
});
