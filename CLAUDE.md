# TranscendedRO

rAthena-based Ragnarok Online server (Renewal).

## Writing custom items

- New custom equipment goes in `db/re/custom/item_db_equip.yml`.
- Set/combo bonuses go in `db/import/item_combos.yml` (loaded after `db/re/item_combos.yml`, untouched by rAthena updates).
- Item data comes from Divine Pride (`https://www.divine-pride.net/database/item/<id>`). Follow the item description text literally; don't second-guess values that look unusual.
- Divine Pride weight is in-game weight; the DB stores it ×10 (in-game 30 → `Weight: 300`).
- Set partner items: use the IDs Divine Pride lists in the set section and map them to AegisNames in our DB. Every combo item must exist in our DB, or the combo fails to load.
- Headgear needs a `View` (sprite ID). Divine Pride doesn't show it, so ask the user for it.

### Fixed interpretations

- "Increase movement speed", "increase moving speed", "movement speed" with no number → `bonus bSpeedRate,25;` (25%).
- "Indestructible in battle" → `bUnbreakableArmor` / `bUnbreakableHelm` / `bUnbreakableShoes` / `bUnbreakableWeapon`, depending on slot.
- "Permanent endure" → `bonus bNoWalkDelay;`
- "Damage taken from all races (include player)" and similar → apply to `RC_All`, `RC_Player_Human` and `RC_Player_Doram`.
- "Normal and Boss monsters" → `Class_All`.
- Arbitrium / Violent Quake skill damage → add `bSkillAtk` for both the skill and its `_ATK` variant.
- Grade bonuses: `.@g = getenchantgrade();` with `ENCHANTGRADE_D/C/B/A`, and set `Gradable: true`.
- Autocast rates are in 1/10 %: 5% → `50`, 100% → `1000`.
