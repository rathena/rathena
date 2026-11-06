# CLAUDE.md — RO Sandbox

A sandbox MMO built on a fork of rAthena (pre-renewal). The goal is a story-driven
redesign of Ragnarok Online: reworked jobs and skills, new resources and crafting.
Read `docs/HANDOFF.md` before starting any task. It holds the systems, the jobs,
the rAthena implementation map and the build order.

## Source of truth

- Design lives in two Google Sheets, linked at the top of `docs/HANDOFF.md`.
  If the sheets and this repo disagree, the sheets win.
- Regenerate import files from the sheets with scripts in `tools/sandbox/`.
  Don't hand-edit generated YAML.

## Build settings

- **Pre-renewal.** Configure with `./configure --enable-prere=yes`. Never build renewal.
- **PACKETVER:** not chosen yet. It must match the client `.exe` date. Ask before setting it.
- Compile with `make clean && make server` (use `-j` up to the machine's core count).

## Where changes go

| Change | Location |
| --- | --- |
| Game data (skills, items, mobs, job stats) | `db/import/` (tracked in git) |
| Server settings, passwords, IPs | `conf/import/` (NOT tracked: secrets) |
| Custom NPC scripts | `npc/custom/sandbox/`, loaded from `npc/scripts_custom.conf` |
| C++ changes | `src/`, kept small and commented with `// [RO Sandbox]` |
| Sheet-to-YAML scripts | `tools/sandbox/` |

Keep edits to upstream files as small as possible, so `git merge upstream/master` stays easy.

## Working rules

1. **Follow the build order** in `docs/HANDOFF.md`, one phase at a time.
2. **Verify with real output.** After each step, show the actual compile output,
   server log lines or command results. A summary is not proof. A phase is done
   only when its check passes on the running server.
3. **Keep it simple.** If a fix adds complexity and still fails, remove the feature
   and report back instead of iterating.
4. **Never touch the database without asking** (no DROP, TRUNCATE or schema changes).
5. **Never commit secrets.** Passwords and IPs stay in `conf/import/`.
6. **Never commit Gravity client files** (GRF, data folder, client `.exe`).
7. **Commit after every working change** with a clear message. Use feature branches; `master` gets merged pull requests.

## In-game testing

Use a GM account (group 99). Useful commands: `@jobchange`, `@allskill`, `@skillpoint`,
`@item`, `@monster`, `@reloadskilldb`, `@reloaditemdb`, `@reloadmobdb`.

## Upstream

- `origin` is this fork. `upstream` is `https://github.com/rathena/rathena`.
- To update: `git fetch upstream && git merge upstream/master`, then rebuild and re-run the phase checks.
