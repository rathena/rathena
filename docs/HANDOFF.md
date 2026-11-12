# RO Sandbox: Vibe Coding Handoff

Rex Irvin Carpen · Nov 6, 2026

## Overview

The design is ready to build: 26 job trees (Novice, 6 first jobs, 19 second jobs), about 30 system rules, and an item database with 369 recipes and 50 custom items, all on a pre-renewal rAthena base.

| Source | What it holds |
| --- | --- |
| [RO\_Skill\_Trees](https://docs.google.com/spreadsheets/d/1hZjgxrIMtovbU2v_QkTTRp4lYQNNunaTGAuN0HEa9RE/edit) | One tab per job (skills, levels, costs, cooldowns, prerequisites, descriptions), Summary, Gameplay Notes (system rules), Crafting, Unique Equipment, Calcs |
| [RO\_Items\_PreRenewal](https://docs.google.com/spreadsheets/d/1jWZez0npq46j8lH682okNJrxiZRB0Izs_GIMvZlyHss/edit) | Items (about 6,170 official items plus custom ones, with a Source column), Recipes (369), Monster Drops (5,045 drops) |
| rAthena master, pre-renewal db | The base data: skill\_tree.yml, skill\_db.yml, item\_db, mob\_db.yml, produce\_db.txt, create\_arrow\_db.yml |

The sheets stay the source of truth: if this doc and a sheet disagree, the sheet wins. Each section below says what to build; the implementation map says where it goes in rAthena, and the build order says when.

## Core systems

These rules apply server-wide and most need source changes, not just data. Full wording is on the Gameplay Notes tab.

| System | Rule |
| --- | --- |
| Two resources | SP is now Stamina: 100 base, +1 per 2 VIT, regenerates 10 per second, limits burst. MP is Mana (the old SP): large pool, slow regen, limits endurance. Some skills cost both. |
| Skill cooldowns | Every skill can have its own cooldown (Cooldown column on each job tab). |
| Skill bar | Only 8 active skills can be equipped at a time. |
| Elite skills | Elites share one 45 s cooldown, plus their own (usually 60 s). Charge elites (Vulcan Arrows, Dance of the Macabre) recharge per charge. Multi-Cast and Overcast never apply to elites. |
| Skill points | 1 point at Job Lv 1, so 50 per 1st and 2nd job. Unused points carry over to the next job. +10 points when an Adept is promoted after transcending. |
| Quest skills | Free, 1 level, fixed power, never a prerequisite. |
| Transcendence | Base Level kept, Job Level resets. +10 all stats, +25% HP and MP, stat and Base Level caps 120. Adept changes job at Job Lv 15: same job unlocks the full tree, a different job keeps both kits. Once only; 3rd job is final. |
| Debuff durations | Soft debuffs curable by items (Silence, Blind, Poison, Curse) may last long. Hard control (stun, freeze, petrify, sleep) stays short with immunity windows. |
| Reductions multiply | DEF, MDEF and damage reductions from different sources multiply: Value × (1 − r1) × (1 − r2)… They can never reach 100%. |
| Flat before percent | Damage = (Base + flat bonuses) × percentage bonuses. |
| Detection | Ruwach and Sight reveal for 3 s, Detect for 1 s; 15 s cooldown (Detect 10 s, 10 Mana). Templar Inquisitors are the detection specialists. |
| Fog of war | Day vision 9x9, night 7x7. Vision skills (Flare Shots and others) share vision with the party. |
| Potion tolerance | Max 100. Every potion adds 10, fading after 10 / 100 / 1,000 s by tier. Over 80: incurable poison, 10% Max HP per second. At 100: potions stop working, poison doubles. Clear White resets it, then blocks potions for 30 s. Launchers: 3 potions per target per 1,000 s. |
| Food | No tolerance. Stomach = 10% of weight limit; a dish fills its ingredients' total weight and digests after 5 minutes. Timed food effects refresh instead of stacking. |
| Heavy armor | Any armor weighing as much as Chain Mail (33) or more. Needs Divine Covenant of the Armor (Templar). |
| Economy | Negotiate replaces Discount and Overcharge (24% each way). Vending EXP set per item. Items bought from a Merchant can't go back to that seller for 2 weeks. Bounties limited to monsters within 10 levels and 10% of next level's EXP. |
| Visual cues | Counterplay skills (Water Moon Reflection, Elemental Marks, stack counters) need loud, readable effects and status icons. |

## Jobs

Every 2nd job now offers real build choices: with 50 points, players learn 27% to 50% of their tree, down from about 60% in classic RO. Job tabs list each skill's new max level, costs, cooldown, prerequisites and description; the Change Notes column says what changed.

| Job | Branch | Tree (points) | Identity on this server |
| --- | --- | --- | --- |
| Novice | — | 70 | Basics; 25 points |
| Swordsman | 1st | 140 | Stance Switch, Bash rework (Fatal Blow), Magnum Break as a Fire Bash |
| Mage | 1st | 140 | Bolt reworks, Spell Mastery, Stone Curse shatter |
| Marksman | 1st | 130 | Renamed Archer; Fates Gamble coins, Trick Shot, quest finishers |
| Acolyte | 1st | 118 | Support base for Priest, Monk, Templar |
| Merchant | 1st | 110 | Negotiate, Pay to Win, Salvage, Gourmet, Recipe Book |
| Thief | 1st | 110 | Hiding with cooldown, Rapid Strikes, Twin Needle |
| Knight | Swordsman | 141 | Lancer, Two-Handed Sword, Two-Handed Axe paths |
| Crusader | Swordsman | 162 | Martyr's Blood stacks, Champion of Light, Great Shield of Fortitude |
| Sword Master | Swordsman | 171 | Dragon Slayer, Northwind, Ocean Moon styles with Momentum, Advances, Focus |
| Wizard | Mage | 176 | Big area magic; Storm Gust freeze-lock guard, Jupitel rework |
| Sage | Mage | 140 | Spell control; Spell Sealer, Magic Exhaust Storm, Primordial Bolt |
| Enchantress | Mage | 180 | Staff battle mage; endows and field combos |
| Hunter | Marksman | 142 | Traps, falcon, tracking; Hunting Ground, Remote Mines |
| Gunslinger | Marksman | 101 | Artillery, Rapid, Sniper gun builds; Arsenal Stance |
| Ranger | Marksman | 130 | Bow and gun soldier; Martial Combat dual wield, Assassinate, PowerShot |
| Priest | Acolyte | 133 | Healer; Penitence, Magnus Exorcismus and Requiem for Souls (elites) |
| Monk | Acolyte | 162 | Classic spheres plus Weapons Master path; Asura Strike elite |
| Templar | Acolyte | 141 | Inquisitor, Battle Cleric, Guardian paths |
| Blacksmith | Merchant | 159 | Forge Master, Battlesmith, Support Smith; Metal Works, Weapon Forging |
| Alchemist | Merchant | 132 | Potions and tolerance, Potion Launchers, Gem Transmutation |
| Bard | Merchant | 136 | Solo ensembles, rhythm skills, Vulcan Arrows |
| Dancer | Merchant | 126 | Solo ensembles, 4-cell whips, Dance of the Macabre |
| Assassin | Thief | 120 | Poison marks, Cloak and Dagger, Soul Destroyer |
| Rogue | Thief | 115 | Mimicry, Envious Eye, Binding Shot, Hate Raid |
| Kage | Thief | 188 | Physical shadow path and Ninjutsu Elemental Marks |

## Items and crafting

All 50 custom items have IDs, prices, weights and effects, and every recipe resolves to a real item. Crafted items are priced at the sum of their ingredients, so crafting never creates zeny from NPCs; the two gold items are priced by the player market instead.

| ID range | Category | Items |
| --- | --- | --- |
| 30000–30011 | Materials and ingredients | Green Gemstone, 9 cooking ingredients, Artificer's Hammer (crafting tool). Egg uses the official Egg (574), so 30001 is unused |
| 30100–30112 | Potions | 12 boosting potions, Clear White |
| 30200–30210 | Equipment | Golden Thief Blade, Hunter's Avarice, Mammon's Golden Axe, Golden Cursed Ring, Blade of Golden Lining, 6 runic weapons |
| 30300–30307 | Recipe pages | One per unique recipe, 0.05% drops (Blade page: Golden Thief Bug, 1%) |
| 30400–30406 | Food | 7 dishes |

| Recipe book | Skill | Recipes |
| --- | --- | --- |
| Alchemist's Formulary | Prepare Potion, Gem Transmutation | 37 |
| Chef's Cookbook | Gourmet | 7 |
| Metalworker's Ledger | Metal Works | 5 |
| Weaponsmith's Codex | Weapon Forging | 52 (all classic forging recipes) |
| Armorer's Codex | Armor Forging | 10 (heavy armor and shields) |
| Tailor's Pattern Book | Weaving and Tailoring | 16 |
| Jeweler's Catalog | Jewelry Crafting | 16 |
| Runic Grimoire | Runic Arts | 7 |
| Fletcher's Notes | Arrow Crafting | 219 (all classic conversions) |

Rules that code must enforce:

- The Recipe Book is free and automatic for the Merchant branch and Marksman branch; basic recipes unlock with skill level, unique ones by using a recipe page.
- Runic weapons consume the card and have no card slots, but can be reforged into Very Very Strong elemental weapons.
- The Artificer's Hammer must be in the inventory but is not used up.
- Drop-rate bonuses multiply (Golden Cursed Ring: x1.25; Blade of Golden Lining: up to x1.0625).
- New drops (ingredients, Green Gemstone, recipe pages) are on the Monster Drops tab; the Items tab's Source column reads from it.

## rAthena implementation map

About half the work is data that rAthena already reads; the rest needs C++ source changes, and four items also need client work. Put data changes in `db/import/` so rAthena updates don't overwrite them.

| Work | Type | Where |
| --- | --- | --- |
| Skill trees and prerequisites | Data | `db/import/skill_tree.yml` |
| Skill levels, costs, cast times, cooldowns | Data | `db/import/skill_db.yml` (has a Cooldown field) |
| New skill effects | Source + client | `src/map/skill.cpp`, `battle.cpp`, `status.cpp`; client skill info and descriptions |
| Custom items | Data + client | `db/import/item_db.yml` (IDs 30000+), item scripts for effects; client `SystemEN/itemInfo_C.lua` |
| Monster drops | Data | `db/import/mob_db.yml` |
| Forging and arrow recipes | Data | `db/import/produce_db.txt`, `db/import/create_arrow_db.yml` (classic forging and arrows already match rAthena) |
| Recipe Book, cooking, recipe pages | Script | NPC script menu, learned recipes as character variables |
| New jobs (Sword Master, Enchantress, Templar, Ranger, Kage), Marksman rename | Source + client | Job IDs and stats in source and `job_stats`; client job names and sprites |
| Stamina resource | Source + client | New stat and regen in `status.cpp` / `pc.cpp`; a client bar |
| Elite shared cooldown, charges, 8-skill bar | Source (+ client for the bar) | `skill.cpp` cooldown handling |
| Multiplicative reductions, flat before percent | Source | Damage formulas in `battle.cpp` |
| Fog of war by day and night | Source | View range is global today (battle config); per-time and shared vision need source |
| Potion tolerance, food stomach | Source or script | Custom status changes in `status.cpp`, or item-script counters |
| Vending EXP, item tagging, bounty limits | Source + script | `vending.cpp`, quest scripts |
| Transcendence rules, skill point carry-over | Source + data | `pc.cpp`, job stats |

File paths are from rAthena master as of late October 2026; confirm them against your checkout before editing.

## Build order

Start with data that rAthena already reads, so the server is playable early, and add source changes one system at a time. Each phase ends with a check on the live server, not a summary.

1. **Base server.** Clean rAthena pre-renewal build, compiles and runs. *Done 2026-11-07.*
   - Check: map server log shows the maps and databases loaded with no errors.
2. **Item database.** Export Items and Monster Drops to `db/import/` item and mob files, and the custom items to the client, with `tools/sandbox/sheet2yaml.py`. *Done 2026-11-08.*
   - Check: `@item 30000` gives a Green Gemstone; a Harpy can drop one.
   - The Recipes tab moved to phase 7: classic forging and arrows already match rAthena, and the rest needs the Recipe Book.
3. **Existing-job skill data.** Levels, costs, cast times, cooldowns and trees for jobs that already exist in rAthena, with `tools/sandbox/skills2yaml.py`. *Done 2026-11-09.*
   - Check: a test Knight sees the new tree and cooldowns in game.
   - Covers Novice, the six 1st jobs and the 13 classic 2nd jobs. Costs wait for phase 6; new skills and reworked effects for phase 8.
   - Until a skill named in New Prerequisites exists, the skill keeps its classic prerequisites. Classic skills a tab doesn't list stay until their replacement exists.
4. **Simple rule changes.** Multiplicative reductions, flat-before-percent, the DEF and MDEF formula, hard-control durations, detection timings. *Done 2026-11-10 (PRs #11 to #17).*
   - Check (replaces the sheet's 72% example, which was for the old formula): Horn Card 35% + Noxious Card 10% gives 42% ranged reduction; Bash Lv 10 on Iron Fist matches the new DEF formula; bosses and MVPs can be stunned and the immunity window stops stun-locks.
   - DEF and MDEF: Mitigation = 0.75 x A / (A + k) + 0.25 x B / (B + k), with k = C x attacker level / defender level (C: player 24, normal monster 500, boss 250, MVP 125). A is armor, B is cards, item effects and buffs. After the percentage, subtract VIT DEF + 0.75 x B + 0.25 x A (magic: INT + VIT/2 + the same). Criticals ignore DEF, but crit chance x (1 - Mitigation / 2). No 99 DEF cap.
   - Hard control (stun, freeze, petrify, sleep) is rescaled from its longest base duration (5 / 30 / 20 / 30 s) to a limit: normal monster 15 s, player 6 s, boss 5 s, MVP 3 s. Bosses and MVPs can be controlled. Each effect leaves an immunity window of 20% of the limit.
   - Durations for Ruwach, Sight and Detect come from the Duration (s) column of RO_Skill_Trees. Detect's repeat reveal is not tested in game yet.
5. **Elite cooldown and 8-skill bar.**
   - Check: one elite puts the others on cooldown; a 9th skill cannot be equipped.
6. **Stamina resource.** The largest change: every skill's SP/MP split depends on it.
   - Check: Stamina regenerates 10 per second and Breathing Mastery adds up to 50.
7. **Crafting scripts.** Recipe Book (and the Recipes tab export: 30 changed or new alchemy recipes, plus every other book), Gourmet and food stomach, potion tolerance, Clear White.
   - Check: a 9th Red Potion inside 10 seconds triggers the poison.
8. **New skills for existing jobs.** One job at a time, starting with the Merchant branch (its crafting is already in place).
9. **New jobs.** Sword Master, Enchantress, Templar, Ranger and Kage, including client job names and sprites.
10. **Fog of war and economy rules.** Day and night vision, vending EXP, item tagging, bounty limits.

Keep the sheets as the source and regenerate the import files from them, rather than hand-editing the YAML.

## Open items

None of these block phases 1 to 4; most are small design calls or cleanups.

- [ ] Give every new skill a numeric skill ID (the sheets have codes like `KN_GUILLOTINE`, not IDs).
- [ ] Fill Base EXP and Job EXP per item on the Items tab, which Vending EXP reads.
- [ ] Define how Sight, Ruwach and other skills extend vision under fog of war.
- [ ] Update two stale Gameplay Notes lines: the Bard and Dancer note still names Rose Whip of Exorcism (now Dance of the Macabre), and the Clear White note still says Witherless Flora and Slim White Potions (now Witherless Rose and Condensed White Potion).
- [ ] Retire or redo the Mage Usual Build tab, which predates the Mage rework.
- [ ] Write the NPC scripts for Jade's gemstone trade and the Jewel Shop (Green Gemstone, Artificer's Hammer).
- [ ] Decide client art for the five new jobs, or reuse existing sprites for a first version.
- [ ] Design 3rd jobs, the final form after transcendence.
- [ ] Draw icons and sprites for the custom items. For now each borrows an official item's look (Items tab, Client Look column).
- [ ] Rename or switch off the client's "[ExampleRO]" tag on custom item names (ROTP setting in `SystemEN/itemInfo.lua`).
- [ ] Check 6 Fletcher's Notes rows whose material names match more than one item (for example, Wooden Mail).
