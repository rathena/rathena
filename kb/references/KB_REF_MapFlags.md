---
kb_id: KB_REF_003
kb_type: reference
kb_category: map_configuration
kb_subcategory: mapflags
kb_keywords: [mapflag, map settings, pvp, gvg, woe, restrictions, noteleport, nowarp, noreturn, nosave, battle, map behavior, setmapflag, removemapflag]
kb_related: [KB_CMD_010, KB_CONF_003]
kb_difficulty: intermediate
kb_version: rAthena_2025
kb_last_updated: 2013-08-30
kb_use_case: [map_configuration, pvp_setup, woe_setup, server_customization]
---

# rAthena Mapflag Reference

Complete reference for all mapflags that control map behavior and restrictions.

## Overview

Mapflags determine how a map behaves in various situations - from teleportation restrictions to PvP modes to weather effects. They are essential for configuring your server's gameplay experience.

**Configuration:**
- Set in NPC scripts using `setmapflag` command
- Remove using `removemapflag` command
- Usually configured in `/npc/mapflag/` directory

**Basic Syntax:**
```c
<map_name>	mapflag	<mapflag_name>{	<parameters>}
```

---

## CATEGORIES

1. [Restrictions](#1-restrictions) - Movement, items, trading
2. [Battle-Related](#2-battle-related) - PvP, GvG, damage
3. [Map Effects](#3-map-effects) - Weather, visuals
4. [Miscellaneous](#4-miscellaneous) - Exp rates, special features

---

## 1. RESTRICTIONS

### noreturn

**Description:** Disables map-warping items that return players to specific locations.

**Blocks:**
- Butterfly Wing (ID 602)
- Yellow/Green/Red/Blue Butterfly Wing (IDs 14582-14585)
- Siege Teleport Scroll (ID 14591)
- Dungeon Teleport Scroll 1/2/3 (IDs 14527, 14581, 12352)
- `warpparty` and `warpguild` script commands (for destinations outside current map)

**Example:**
```c
prontera	mapflag	noreturn
```

**Use Case:** WoE castles, dungeons, event maps where you don't want easy exits.

---

### noteleport

**Description:** Disables ALL teleportation methods within a map.

**Blocks:**
- Fly Wing (ID 601)
- Giant Fly Wing (ID 12212)
- Skills: AL_TELEPORT, TK_HIGHJUMP, SC_DIMENSIONDOOR
- Skills won't teleport targets: RG_INTIMIDATE, NPC_EXPULSION, CG_TAROTCARD
- Script command `warp` with "Random" as destination
- Script command `warpwaitingpc` with "SavePoint" as destination
- Script command `unitwarp` for players
- Atcommand `@jump`

**Example:**
```c
pvp_n_1-1	mapflag	noteleport
```

**Use Case:** PvP arenas, boss rooms, areas where teleporting would be unfair or break gameplay.

---

### nowarp

**Description:** Disables warping FROM a map (prevents leaving).

**Blocks:**
- Script commands `warpparty` and `warpguild` won't warp players FROM nowarp maps
- Atcommands: `@warp`, `@go`, `@load`, `@jump`
- Atcommands: `@partyrecall`, `@guildrecall`, `@recallall` won't pull players from nowarp maps
- Skill GD_EMERGENCYCALL won't warp players from nowarp maps
- Unit UNT_CALLFAMILY won't warp players from nowarp maps

**Example:**
```c
guild_vs1	mapflag	nowarp
```

**Use Case:** Lock players in area (event maps, special instances, jail).

**Note:** Players can still be warped TO the map, just not FROM it.

---

### nowarpto

**Description:** Disables warping TO a map (prevents entering).

**Blocks:**
- Atcommands: `@warp`, `@go`, `@load`, `@jump` cannot target this map
- Atcommands: `@partyrecall`, `@guildrecall`, `@recallall` cannot target this map
- Command `/memo` disabled
- Skill GD_EMERGENCYCALL disabled (if flag 16 set in `/conf/battle/skill.conf`) - doesn't work for gvg_castle maps

**Example:**
```c
guild_vs2	mapflag	nowarpto
```

**Use Case:** Restricted access maps, GM-only areas, prevent unauthorized entry.

---

### nogo

**Description:** Disables usage of `@go` command on the map.

**Example:**
```c
prontera	mapflag	nogo
```

**Use Case:** Prevent players from using `@go` shortcut to leave area.

---

### nosave <map_name>,<x>,<y>

**Description:** Disables auto-saving on map. Players who log off here will be warped to specified location on login.

**Parameters:**
- `<map_name>`: Map to respawn at (use `SavePoint` for player's save point)
- `<x>,<y>`: Coordinates (optional with SavePoint)

**Examples:**
```c
// Respawn at SavePoint
guild_vs1	mapflag	nosave	SavePoint

// Respawn at specific location
pvp_n_1-1	mapflag	nosave	prontera,156,191
```

**Use Case:** Temporary areas, dungeons, instances where you don't want save point changed.

---

### nomemo

**Description:** Disables saving warp point and marriage skills.

**Blocks:**
- `/memo` command (can't save warp point)
- Marriage skills: WE_CALLPARTNER, WE_CALLPARENT, WE_CALLBABY

**Example:**
```c
gef_dun00	mapflag	nomemo
```

**Use Case:** Dungeons, instances, areas where warp points would break progression.

---

### noitemconsumption

**Description:** Disables usage of ALL items on the map.

**Blocks:**
- All consumable items
- Equipment changes
- Item usage

**Example:**
```c
pvp_y_1-1	mapflag	noitemconsumption
```

**Use Case:** Pure skill-based PvP, special challenge maps, no-item events.

**Note:** Can be bypassed with PC_PERM_ITEM_UNCONDITIONAL permission.

---

### notrade

**Description:** Disables trading between players on the map.

**Example:**
```c
prontera	mapflag	notrade
```

**Use Case:** Prevent trading in specific areas, reduce scam potential in town centers.

---

### nodrop

**Description:** Disables dropping items on the map.

**Exception:** Items may still drop if inventory is full and `item_flooritem_check` is disabled in `/conf/battle/items.conf`.

**Example:**
```c
prontera	mapflag	nodrop
```

**Use Case:** Keep maps clean, prevent item drops in towns, anti-grief measure.

---

### noloot / nomobloot / nomvploot

**Description:** Disables monsters from dropping items.

- `noloot`: All monsters (same as nomobloot + nomvploot)
- `nomobloot`: Normal monsters don't drop
- `nomvploot`: MVP monsters don't drop

**Note:** Looted items (from Gank/Steal skills) will still drop.

**Examples:**
```c
// No drops at all
prt_fild08	mapflag	noloot

// Only normal mobs don't drop (MVPs still do)
pay_fild01	mapflag	nomobloot

// MVPs don't drop (normal mobs still do)
mjolnir_01	mapflag	nomvploot
```

**Use Case:** Training maps, pure exp farming, reduce item database load.

---

### noexp / nobaseexp / nojobexp

**Description:** Disables gaining experience from monsters.

- `noexp`: No base AND job exp (same as nobaseexp + nojobexp)
- `nobaseexp`: No base experience
- `nojobexp`: No job experience

**Includes:** MVP bonuses also disabled.

**Examples:**
```c
// No experience at all
pvp_y_1-1	mapflag	noexp

// No base exp (still get job exp)
prt_fild08	mapflag	nobaseexp

// No job exp (still get base exp)
pay_fild01	mapflag	nojobexp
```

**Use Case:** PvP maps, testing areas, item farming without leveling.

---

### nopenalty / noexppenalty / nozenypenalty

**Description:** Disables loss of exp/zeny upon death.

- `nopenalty`: No exp AND zeny loss (same as noexppenalty + nozenypenalty)
- `noexppenalty`: No exp loss on death
- `nozenypenalty`: No zeny loss on death

**Notes:**
- `noexppenalty` also affects pets
- Skills PR_REDEMPTIO and LG_INSPIRATION won't deduct EXP
- `nozenypenalty` only applies if `zeny_penalty` enabled in `/conf/battle/exp.conf`

**Examples:**
```c
// No penalties at all
prontera	mapflag	nopenalty

// Only exp protected
pvp_n_1-1	mapflag	noexppenalty

// Only zeny protected
gef_fild00	mapflag	nozenypenalty
```

**Use Case:** Newbie areas, PvP maps, event maps, low-penalty zones.

---

### nochat

**Description:** Disables chatroom creation on the map.

**Example:**
```c
pvp_n_1-1	mapflag	nochat
```

**Use Case:** Reduce spam in busy areas, PvP focus, event maps.

---

### novending

**Description:** Disables shop creation from MC_VENDING skill.

**Example:**
```c
prontera	mapflag	novending
```

**Use Case:** Designated vending areas, reduce map clutter, prevent vendor spam.

---

### nobuyingstore

**Description:** Disables shop creation from ALL_BUYING_STORE skill.

**Example:**
```c
prontera	mapflag	nobuyingstore
```

**Use Case:** Designated buying areas, prevent spam, organize economy.

---

### nousecart

**Description:** Disables cart usage on the map.

**Example:**
```c
aldebaran	mapflag	nousecart
```

**Use Case:** Reduce sprite load, specific area restrictions, roleplay purposes.

---

### noskill

**Description:** Disables ALL skill usage on the map.

**Example:**
```c
prontera	mapflag	noskill
```

**Use Case:** Safe zones, event maps, roleplaying areas, extreme restrictions.

**Warning:** Very restrictive - disables ALL skills including buffs.

---

### restricted <zone>

**Description:** Disables certain items and skills based on zone number.

**Zone Databases:**
- `/db/(pre-)re/item_noequip.txt` - Item restrictions
- `/db/(pre-)re/skill_nocast_db.txt` - Skill restrictions

**Restricted Zones:**
- `1` - Aldebaran Turbo Track
- `2` - Jail
- `3` - Izlude Battle Arena
- `4` - WoE:SE Maps
- `5` - Sealed Shrine
- `6` - Instances (Endless Tower, Orc's Memory, Nidhoggr's Instance)
- `7` - Towns
- `8` - WOE:TE Dungeons

**Examples:**
```c
// Restrict town items/skills
prontera	mapflag	restricted	7

// Restrict WoE items/skills
prtg_cas01	mapflag	restricted	4
```

**Use Case:** Balanced competitive areas, specific gameplay rules, item/skill bans.

---

### monster_noteleport

**Description:** Prevents monsters from teleporting on the map.

**Blocks:**
- Monster teleportation skills
- RG_INTIMIDATE won't teleport monsters

**Example:**
```c
boss_map	mapflag	monster_noteleport
```

**Use Case:** Boss fights, prevent mobs escaping, specific monster behavior control.

---

### nobranch

**Description:** Disables monster-spawning items on the map.

**Blocks:**
- Dead Branch (ID 604)
- Bloody Branch (ID 12103)
- Poring Box (ID 12109)
- Red Pouch (ID 12024)

**Note:** Items can be modified in `/db/(pre-)re/item_flag.txt`.

**Special:** If `mob_warp` enabled with flag 4 in `/conf/battle/monster.conf`, also prevents mobs being warped onto the map (except slaves).

**Example:**
```c
prontera	mapflag	nobranch
```

**Use Case:** Towns, prevent griefing, controlled spawn areas.

---

### noicewall

**Description:** Disables skill WZ_ICEWALL on the map.

**Example:**
```c
pvp_n_1-1	mapflag	noicewall
```

**Use Case:** PvP balance, prevent blocking, reduce skill spam.

---

### nosunmoonstarmiracle

**Description:** Disables Star Gladiator's "Solar, Lunar, and Stellar Miracle" from occurring.

**Example:**
```c
prontera	mapflag	nosunmoonstarmiracle
```

**Use Case:** Prevent exploit areas, balance specific maps.

---

### forcemineffect

**Description:** Forces simpler skill effects (like `/mineffect` command).

**Example:**
```c
pvp_n_1-1	mapflag	forcemineffect
```

**Use Case:** Performance optimization, reduce lag in busy areas, cleaner visuals.

---

### nolockon

**Description:** Disables attacking another player without holding shift or using `/ns`.

**Example:**
```c
prontera	mapflag	nolockon
```

**Use Case:** Prevent accidental PvP, safe zones, town protection.

---

### nocommand <group_level>

**Description:** Disables commands on the map.

**Parameters:**
- No parameter: Disables ALL commands for everyone
- `<group_level>`: Only disables for players with group level BELOW this value

**Examples:**
```c
// Disable all commands for everyone
pvp_n_1-1	mapflag	nocommand

// Disable commands for group level below 50 (normal players)
prontera	mapflag	nocommand	50
```

**Use Case:** Prevent command abuse, fair play areas, restricted zones.

---

### nomapchannelautojoin

**Description:** Stops players from automatically joining #map channel.

**Requirements:**
- Map channels must be enabled
- `map_local_channel_autojoin` must be true in `/conf/channels.conf`

**Example:**
```c
prontera	mapflag	nomapchannelautojoin
```

**Use Case:** Reduce chat spam, private areas, roleplay control.

---

### notomb

**Description:** Disables MVP tombs from appearing on the map.

**Example:**
```c
boss_map	mapflag	notomb
```

**Use Case:** Clean boss areas, prevent tomb spam, aesthetic control.

---

### nocostume

**Description:** Disables costume sprites on the map.

**Notes:**
- Only disables visual sprites, NOT item effects
- If player logs out on nocostume map, costumes won't show in character server either

**Example:**
```c
prontera	mapflag	nocostume
```

**Use Case:** Roleplay immersion, aesthetic control, visual consistency.

---

### norenewaldroppenalty

**Description:** Disables renewal drop rate penalty due to level difference.

**Example:**
```c
prt_fild08	mapflag	norenewaldroppenalty
```

**Use Case:** Training maps, allow high-level farming, balanced drop rates.

---

### norenewalexppenalty

**Description:** Disables renewal experience penalty due to level difference.

**Example:**
```c
prt_fild08	mapflag	norenewalexppenalty
```

**Use Case:** Training maps, allow high-level farming, balanced exp gain.

---

### nopetcapture

**Description:** Disables ability to capture pets on the map.

**Example:**
```c
pvp_n_1-1	mapflag	nopetcapture
```

**Use Case:** PvP maps, boss maps, special areas.

---

### nobank

**Description:** Disables Bank system on the map.

**Example:**
```c
pvp_n_1-1	mapflag	nobank
```

**Use Case:** Restricted areas, prevent banking exploits.

---

### norodex

**Description:** Disables RODex (mail system) on the map.

**Example:**
```c
pvp_n_1-1	mapflag	norodex
```

**Use Case:** Event maps, prevent mail abuse, focus gameplay.

---

## 2. BATTLE-RELATED

### pvp / pvp_noparty / pvp_noguild / pvp_nocalcrank

**Description:** Enables Player vs. Player mode with damage adjustments.

- `pvp`: Standard PvP mode
- `pvp_noparty`: Ignore party alliances (can hit party members)
- `pvp_noguild`: Ignore guild alliances (can hit guild members)
- `pvp_nocalcrank`: Disable PvP ranking calculation

**Examples:**
```c
// Standard PvP
pvp_y_1-1	mapflag	pvp

// Free-for-all (ignore party)
pvp_n_1-1	mapflag	pvp
pvp_n_1-1	mapflag	pvp_noparty

// Full FFA (ignore party and guild)
pvp_n_2-2	mapflag	pvp
pvp_n_2-2	mapflag	pvp_noparty
pvp_n_2-2	mapflag	pvp_noguild

// PvP without ranking
pvp_y_2-2	mapflag	pvp
pvp_y_2-2	mapflag	pvp_nocalcrank
```

**Use Case:** PvP arenas, deathmatch areas, team battles, ranked matches.

---

### pvp_nightmaredrop <id>,<type>,<rate>

**Description:** Causes players to drop items upon death.

**Parameters:**
- `<id>`: Item ID or "random"
- `<type>`: "inventory", "equip", or "all"
- `<rate>`: Drop chance (10000 = 100%)

**Examples:**
```c
// Drop random item from inventory (50% chance)
pvp_n_1-1	mapflag	pvp_nightmaredrop	random,inventory,5000

// Drop specific item (100% chance)
pvp_n_1-1	mapflag	pvp_nightmaredrop	501,inventory,10000

// Drop random equipment (10% chance)
pvp_n_1-1	mapflag	pvp_nightmaredrop	random,equip,1000

// Drop anything (25% chance)
pvp_n_1-1	mapflag	pvp_nightmaredrop	random,all,2500
```

**Use Case:** Hardcore PvP, high-risk areas, unique game modes.

**Note:** Does NOT require PvP mapflag to be set (works anywhere).

---

### gvg / gvg_noparty / gvg_castle / gvg_dungeon / gvg_te / gvg_te_castle

**Description:** Enables Guild vs. Guild mode with damage adjustments.

- `gvg`: Standard GvG mode
- `gvg_noparty`: Ignore party alliances
- `gvg_castle`: Guild castle (GvG only active during WoE)
- `gvg_dungeon`: Guild dungeon (warp out after 2 deaths)
- `gvg_te`: WOE:TE area
- `gvg_te_castle`: WOE:TE castle (special restrictions)

**Examples:**
```c
// Standard GvG
guild_vs1	mapflag	gvg

// WoE Castle
prtg_cas01	mapflag	gvg_castle

// Guild Dungeon
gld_dun01	mapflag	gvg_dungeon

// WoE:TE
te_prtcas01	mapflag	gvg_te
te_prtcas01	mapflag	gvg_te_castle
```

**Use Case:** War of Emperium, guild wars, guild dungeons, competitive guild content.

---

### battleground {<type>}

**Description:** Enables Battlegrounds mode with damage adjustments.

**Parameters:**
- `1` (default): Nothing
- `2`: Show scoreboard

**Examples:**
```c
// Basic BG
bat_c01	mapflag	battleground

// BG with scoreboard
bat_c02	mapflag	battleground	2
```

**Use Case:** Battleground arenas, team-based competitive content.

---

### partylock / guildlock

**Description:** Prevents alteration of parties/guilds on the map.

**Blocks:**
- Creating
- Leaving
- Inviting
- Expelling
- Breaking
- Changing leaders

**Notes:**
- `partylock`: Still allows changing party options
- `guildlock`: Also blocks guild alliance changes

**Examples:**
```c
// Lock party changes
pvp_n_1-1	mapflag	partylock

// Lock guild changes
guild_vs1	mapflag	guildlock
```

**Use Case:** Competitive events, prevent team switching mid-match, tournament fairness.

---

### skill_damage {<skill_name>,<caster>,<SKILLDMG_PC>,{<SKILLDMG_MOB>,{<SKILLDMG_BOSS>,{<SKILLDMG_OTHER>}}}}

**Description:** Adjusts skill damage on the map.

**Parameters:**
- `skill_name`: Skill name from skill_db.yml (or "all" for all skills)
- `caster`: Caster type(s) - bitmask
  - `BL_PC` = Player
  - `BL_MOB` = Monster
  - `BL_PET` = Pet
  - `BL_HOM` = Homunculus
  - `BL_MER` = Mercenary
  - `BL_ELEM` = Elemental
- `damage`: Percent adjustment (-100 to 100000)
  - `SKILLDMG_PC` = against player
  - `SKILLDMG_MOB` = against normal monster
  - `SKILLDMG_BOSS` = against boss monster
  - `SKILLDMG_OTHER` = against other (hom/merc/pet/elem)

**Note:** Can also use `db/skill_damage_db.txt` for Map type 16.

**Examples:**
```c
// Reduce all player skill damage by 50% in PvP
pvp_n_1-1	mapflag	skill_damage	all,BL_PC,50

// Increase Bash damage against players by 200%
pvp_n_1-1	mapflag	skill_damage	SM_BASH,BL_PC,200

// Disable all damage to monsters
prt_fild08	mapflag	skill_damage	all,BL_MOB,0
```

**Use Case:** PvP balance, custom damage rules, skill adjustments, unique game modes.

---

### skill_duration <skill_name>,<percentage>

**Description:** Sets trap-type skill duration to percentage of original.

**Example:**
```c
// Makes HT_ANKLESNARE last 4x longer
prtg_cas01	mapflag	skill_duration	HT_ANKLESNARE,400

// Makes all traps last half as long
pvp_n_1-1	mapflag	skill_duration	all,50
```

**Use Case:** Balance trap skills, WoE adjustments, custom gameplay.

---

### invincible_time <duration>

**Description:** Sets invincibility duration (in ms) when player loads onto map.

**Cancelled By:**
- Player walking
- Player interacting (any action)

**Default:** Uses `player_invincible_time` from `/conf/battle/player.conf` if not specified.

**Example:**
```c
// 5 seconds invincibility on spawn
pvp_n_1-1	mapflag	invincible_time	5000

// 10 seconds invincibility
guild_vs1	mapflag	invincible_time	10000
```

**Use Case:** Prevent spawn camping, fair respawn, give players time to orient.

---

## 3. MAP EFFECTS

### Weather Effects

**Available Effects:**
- `clouds` - Cloudy sky
- `clouds2` - Darker clouds
- `fireworks` - Fireworks display
- `fog` - Foggy atmosphere
- `leaves` - Falling leaves
- `sakura` - Falling cherry blossoms
- `snow` - Snowfall

**Examples:**
```c
prontera	mapflag	sakura
lutie	mapflag	snow
morocc	mapflag	fog
```

**Use Case:** Atmosphere, seasonal events, aesthetic enhancement, immersion.

---

### nightenabled

**Description:** Displays night mode effects on the map.

**Example:**
```c
prt_fild08	mapflag	nightenabled
```

**Use Case:** Most outdoor maps, day/night cycle, atmosphere.

**Note:** Used on most outdoor maps by default.

---

## 4. MISCELLANEOUS

### town

**Description:** Marks map as a town.

**Effects:**
- Allows mail access
- Disables kill stealing

**Example:**
```c
prontera	mapflag	town
```

**Use Case:** Major cities, safe zones, social hubs.

---

### reset

**Description:** Allows usage of Neuralizer (ID 12213).

**Example:**
```c
prontera	mapflag	reset
```

**Use Case:** Stat/skill reset areas, convenience zones.

---

### bexp <rate> / jexp <rate>

**Description:** Changes base/job experience rates on the map.

**Parameters:**
- `<rate>`: Percentage (100 = 1x, 200 = 2x, 50 = 0.5x)
- Supports negative values to reduce EXP
- Takes into account `base_exp_rate` and `job_exp_rate` from `/conf/battle/exp.conf`

**Examples:**
```c
// Double base exp
prt_fild08	mapflag	bexp	200

// Triple job exp
pay_fild01	mapflag	jexp	300

// Half base exp
gef_dun00	mapflag	bexp	50

// No base exp (alternative to nobaseexp)
pvp_n_1-1	mapflag	bexp	0
```

**Use Case:** Training areas, bonus zones, event maps, penalty areas.

---

### loadevent

**Description:** Triggers label "OnPCLoadMapEvent" when players enter the map.

**Triggered By:**
- Entering the map
- Teleporting within the map

**Example:**
```c
prontera	mapflag	loadevent
```

**Script Example:**
```c
-	script	LoadEventExample	-1,{
OnPCLoadMapEvent:
    if (strcharinfo(3) == "prontera") {
        mes "Welcome to Prontera!";
        close;
    }
    end;
}
```

**Use Case:** Welcome messages, buff application, entrance checks, custom events.

**See Also:** `/doc/script_commands.txt` for more details.

---

### allowks

**Description:** Allows kill stealing on the map (renders `@noks` command useless).

**Example:**
```c
prt_fild08	mapflag	allowks
```

**Use Case:** Free-for-all farming, competitive areas, no kill steal protection.

---

### autotrade

**Description:** Allows `@autotrade` command on the map.

**Requirements:**
- Only applies if `at_mapflag` enabled in `/conf/battle/misc.conf`
- Otherwise, @autotrade enabled on all maps by default

**Example:**
```c
prontera	mapflag	autotrade
```

**Use Case:** Designated vending areas, market zones.

---

### hidemobhpbar

**Description:** Hides monster HP bar on the map.

**Example:**
```c
boss_map	mapflag	hidemobhpbar
```

**Use Case:** Aesthetic preference, mystery boss fights, cleaner visuals.

**Note:** Ignores `monster_hp_bars_info` config value.

---

### specialpopup <popup_id>

**Description:** Displays special popup when player enters the map.

**Example:**
```c
prontera	mapflag	specialpopup	1
```

**Use Case:** Announcements, warnings, information displays.

**See Also:** Script command "specialpopup" for popup types.

---

## SCRIPT COMMANDS

### setmapflag("<map_name>", <mapflag>{, <value>})

**Description:** Sets a mapflag on a map dynamically.

**Examples:**
```c
// Enable PvP
setmapflag("prontera", mf_pvp);

// Set nosave
setmapflag("prontera", mf_nosave, "SavePoint");

// Set bexp to 200%
setmapflag("prt_fild08", mf_bexp, 200);
```

---

### removemapflag("<map_name>", <mapflag>)

**Description:** Removes a mapflag from a map dynamically.

**Example:**
```c
// Disable PvP
removemapflag("prontera", mf_pvp);

// Remove noteleport
removemapflag("guild_vs1", mf_noteleport);
```

---

### getmapflag("<map_name>", <mapflag>)

**Description:** Checks if a mapflag is set on a map.

**Returns:** 1 if set, 0 if not set.

**Example:**
```c
if (getmapflag("prontera", mf_pvp)) {
    mes "This is a PvP map!";
} else {
    mes "This is not a PvP map.";
}
```

---

## MAPFLAG CONSTANTS

Use these constants in scripts:

```c
mf_nomemo, mf_noteleport, mf_nosave, mf_nobranch, mf_noexppenalty, mf_nozenypenalty,
mf_notrade, mf_noskill, mf_nowarp, mf_partylock, mf_noicewall, mf_snow, mf_fog,
mf_sakura, mf_leaves, mf_clouds, mf_clouds2, mf_fireworks, mf_gvg_castle, mf_gvg,
mf_pvp, mf_pvp_noparty, mf_pvp_noguild, mf_loadevent, mf_nochat, mf_noexppenalty,
mf_noreturn, mf_nogo, mf_nomemo, mf_pvp_nocalcrank, mf_gvg_te, mf_gvg_te_castle,
mf_battleground, mf_reset, mf_guildlock, mf_town, mf_autotrade, mf_allowks,
mf_monster_noteleport, mf_pvp_nightmaredrop, mf_restricted, mf_nocommand,
mf_nodrop, mf_jexp, mf_bexp, mf_novending, mf_nopenalty, mf_gvg_noparty,
mf_noexppenalty, mf_nozenypenalty, mf_nightenabled, mf_nobaseexp, mf_nojobexp,
mf_nomobloot, mf_nomvploot, mf_noreturn, mf_nowarpto, mf_nightmaredrop,
mf_noexp, mf_noitemconsumption, mf_nosunmoonstarmiracle, mf_nomapchannelautojoin,
mf_nousecart, mf_nolockon, mf_notomb, mf_nocostume, mf_norenewaldroppenalty,
mf_norenewalexppenalty, mf_noloot, mf_nopetcapture, mf_nobuyingstore,
mf_skill_damage, mf_skill_duration, mf_invincible_time, mf_nobank, mf_norodex,
mf_hidemobhpbar, mf_specialpopup
```

---

## COMMON MAPFLAG COMBINATIONS

### PvP Arena (Fair)
```c
pvp_n_1-1	mapflag	pvp
pvp_n_1-1	mapflag	pvp_noparty
pvp_n_1-1	mapflag	pvp_noguild
pvp_n_1-1	mapflag	noteleport
pvp_n_1-1	mapflag	nowarp
pvp_n_1-1	mapflag	nopenalty
pvp_n_1-1	mapflag	nosave	SavePoint
pvp_n_1-1	mapflag	invincible_time	5000
```

### WoE Castle
```c
prtg_cas01	mapflag	gvg_castle
prtg_cas01	mapflag	noteleport
prtg_cas01	mapflag	nowarp
prtg_cas01	mapflag	noreturn
prtg_cas01	mapflag	nobranch
prtg_cas01	mapflag	nomemo
prtg_cas01	mapflag	nosave	SavePoint
prtg_cas01	mapflag	skill_duration	HT_ANKLESNARE,400
```

### Town (Safe Zone)
```c
prontera	mapflag	town
prontera	mapflag	nomemo
prontera	mapflag	nobranch
prontera	mapflag	noteleport
prontera	mapflag	novending
prontera	mapflag	nobuyingstore
prontera	mapflag	nodrop
```

### Training Area (High EXP)
```c
prt_fild08	mapflag	bexp	300
prt_fild08	mapflag	jexp	300
prt_fild08	mapflag	nopenalty
prt_fild08	mapflag	norenewaldroppenalty
prt_fild08	mapflag	norenewalexppenalty
```

### Boss Map
```c
boss_map	mapflag	nomemo
boss_map	mapflag	noteleport
boss_map	mapflag	nobranch
boss_map	mapflag	monster_noteleport
boss_map	mapflag	notomb
boss_map	mapflag	hidemobhpbar
```

---

## BEST PRACTICES

1. **Test Thoroughly:** Always test mapflag combinations in development environment
2. **Document Changes:** Comment your mapflag configurations
3. **Consistency:** Use similar mapflag sets for similar map types
4. **Performance:** Use `forcemineffect` on busy/laggy maps
5. **Balance:** Consider player experience when setting restrictions
6. **Security:** Use `restricted` zones for competitive/balanced areas
7. **Organization:** Keep mapflag configs organized in `/npc/mapflag/` directory

---

## TROUBLESHOOTING

**Problem:** Players can still teleport despite noteleport
- **Solution:** Check for GM permissions (any_warp), items with special flags

**Problem:** Mapflags not applying
- **Solution:** Reload scripts with `@reloadscript`, check syntax, verify map name

**Problem:** PvP damage seems wrong
- **Solution:** Check battle config files, verify pvp mapflag set, check skill_damage adjustments

**Problem:** Players respawn in wrong location with nosave
- **Solution:** Verify map name spelling, check coordinates, ensure SavePoint is capitalized

---

## RELATED FILES

- `/npc/mapflag/` - Mapflag configuration scripts
- `/conf/battle/` - Battle configuration files
- `/doc/script_commands.txt` - Script command reference
- `/db/(pre-)re/skill_nocast_db.txt` - Restricted zones skill config
- `/db/(pre-)re/item_noequip.txt` - Restricted zones item config

---

## SEE ALSO

- **KB_CMD_010:** Map Script Commands
- **KB_CONF_003:** Battle Configuration
- **KB_EXAMPLE_003:** Mapflag Usage Examples

---

*Last Updated: 2013-08-30*
*rAthena Documentation*
