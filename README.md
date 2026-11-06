# RO Sandbox

A game-development sandbox built on a fork of [rAthena](https://github.com/rathena/rathena),
the open-source Ragnarok Online server emulator. It redesigns pre-renewal RO into a more
directed, less grind-heavy game.

## What's changing

- **Two resources:** Stamina for physical skills, Mana for magic. Some skills cost both.
- **Skill cooldowns**, an 8-skill bar and shared-cooldown Elite skills.
- **Reworked jobs:** Archer becomes Marksman. Bard and Dancer move to the Merchant branch.
  Five new 2nd jobs: Sword Master, Enchantress, Templar, Ranger and Kage.
- **Crafting:** recipe books, cooking, potion tolerance and custom equipment.
- **Transcendence rework:** multiclass Adepts.

The full design and build plan is in [`docs/HANDOFF.md`](docs/HANDOFF.md).

## Status

| Phase | Status |
| --- | --- |
| 1. Base server (clean pre-renewal build) | Not started |
| 2. Item database | Not started |
| 3. Existing-job skill data | Not started |

## Repo layout

- `db/import/`: this project's game data, layered on top of rAthena's
- `npc/custom/sandbox/`: custom NPC scripts
- `tools/sandbox/`: scripts that turn the design sheets into import files
- `docs/`: design and handoff notes

## Credits and license

Built on rAthena, which is licensed under the GNU GPL v3 (see [`LICENSE`](LICENSE)).
rAthena's original readme is kept as [`README.rathena.md`](README.rathena.md).
Ragnarok Online is a trademark of Gravity Co., Ltd. No game client files are included.
