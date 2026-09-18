# Test fixtures

Five small GRF archives used by `tests/grf-header.test.js`. Total size under 2 KB, so CI never needs a
real Ragnarok client.

| File | Bytes | What it is |
| --- | --- | --- |
| `with-files.grf` | 655 | Valid v0x200 archive, 7 entries |
| `with-files-v300.grf` | 669 | Valid v0x300 archive, 7 entries |
| `incorrect-version.grf` | 46 | Valid signature, version `0x103` — must be rejected |
| `not-grf.grf` | 57 | Plain text, wrong signature — must be rejected |
| `corrupted.grf` | 0 | Empty file — must be rejected |

## Provenance

Copied from the `data/` directory of [FranciscoWallison/grf-loader](https://github.com/FranciscoWallison/grf-loader),
the source repository of `@chicowall/grf-loader`, this project's own GRF dependency. That project is **MIT
licensed** and the fixtures are tracked in its git history.

They are **synthetic**: their entries are named `raw`, `compressed`, `corrupted`, `compressed-des-header` and
similar. They contain no Ragnarok Online game assets, so nothing here is Gravity's copyrighted content and
these files can be redistributed with this repository.

`with-files-v300.grf` matters in particular. The v0x300 header layout differs from v0x200 in three ways —
64-bit table offset, literal file count, and an extra Int32 before the file table — and the validator used to
read every field with the v0x200 layout. Against the code before that fix this fixture fails with
"Failed to read/parse compacted file table"; against the fixed code it loads. It is the only regression guard
for that path.
