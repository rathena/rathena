---
kb_id: KB_REF_009
kb_type: reference
kb_category: server_configuration
kb_subcategory: battle_mechanics
kb_keywords: [battle config, server rates, exp rate, drop rate, game mechanics, balance, server customization, conf/battle, rates, gameplay]
kb_related: [KB_REF_004, KB_CONF_001]
kb_difficulty: intermediate
kb_version: rAthena_2025
kb_last_updated: 2025-10-23
kb_use_case: [server_setup, balance, customization, rates_adjustment]
---

# rAthena Battle Configuration Reference

Essential battle configuration settings for customizing server mechanics and rates.

## Overview

Battle configuration files control game mechanics, rates, and behavior. These settings are in `/conf/battle/` directory and can be overridden in `/conf/import/battle_conf.txt`.

**Configuration Location:**
- Main files: `/conf/battle/*.conf`
- Custom overrides: `/conf/import/battle_conf.txt` ✓ (recommended)

**How to Apply:**
1. Edit `/conf/import/battle_conf.txt`
2. Restart map server (or use `@reloadbattleconf`)

---

## TABLE OF CONTENTS

1. [Experience & Leveling](#1-experience--leveling)
2. [Drop Rates](#2-drop-rates)
3. [Player Mechanics](#3-player-mechanics)
4. [Monster Mechanics](#4-monster-mechanics)
5. [Party & Guild](#5-party--guild)
6. [PvP & WoE](#6-pvp--woe)
7. [Items & Equipment](#7-items--equipment)
8. [Skills](#8-skills)
9. [Death Penalties](#9-death-penalties)
10. [Misc Settings](#10-misc-settings)

---

## 1. EXPERIENCE & LEVELING

### exp.conf

```conf
// Base experience rate (100 = 1x, 1000 = 10x, 10000 = 100x)
base_exp_rate: 100

// Job experience rate
job_exp_rate: 100

// MVP experience rate
mvp_exp_rate: 100

// Quest experience rate
quest_exp_rate: 100

// Max base level
max_base_level: 99
max_base_level_1_2: 99
max_base_level_3: 175
max_base_level_4: 200

// Max job level
max_job_level: 50
max_job_level_1_2: 50
max_job_level_3: 70
max_job_level_4: 60

// Death penalty
death_penalty_type: 1
death_penalty_base: 100
death_penalty_job: 100

// Resurrection penalty
resurrection_exp: 0

// Zeny penalty on death (100 = 1%)
zeny_penalty: 0
```

**Common Rates:**
- 1x: base_exp_rate: 100
- 5x: base_exp_rate: 500
- 10x: base_exp_rate: 1000
- 50x: base_exp_rate: 5000
- 100x: base_exp_rate: 10000

**Example Override:**
```conf
// 10x rates server
base_exp_rate: 1000
job_exp_rate: 1000
mvp_exp_rate: 1000
```

---

## 2. DROP RATES

### drops.conf

```conf
// Item drop rates (100 = 1x, 200 = 2x)
item_rate_common: 100          // Common items
item_rate_common_boss: 100     // Common from boss
item_rate_heal: 100            // Healing items
item_rate_heal_boss: 100       // Healing from boss
item_rate_use: 100             // Usable items
item_rate_use_boss: 100        // Usable from boss
item_rate_equip: 100           // Equipment
item_rate_equip_boss: 100      // Equipment from boss
item_rate_card: 100            // Cards
item_rate_card_boss: 100       // Cards from boss

// MVP item rates
item_rate_mvp: 100

// Treasure box rates
item_rate_treasure: 100

// Drop rate affected by level difference (Renewal)
// yes = enabled, no = disabled
drops_by_luk: 0

// Minimum drop rate (100 = 1%)
drops_by_luk2: 0
```

**Common Configurations:**
```conf
// 5x drop rates
item_rate_common: 500
item_rate_heal: 500
item_rate_use: 500
item_rate_equip: 500
item_rate_card: 100  // Keep cards rare

// Double boss drops
item_rate_common_boss: 200
item_rate_equip_boss: 200
item_rate_card_boss: 200
```

---

## 3. PLAYER MECHANICS

### player.conf

```conf
// HP/SP modifiers
hp_rate: 100
sp_rate: 100

// Max stats
max_parameter: 99               // Max stat (STR/AGI/VIT/INT/DEX/LUK)
max_baby_parameter: 80          // Max stat for baby classes
max_third_parameter: 130        // Max stat for 3rd jobs
max_fourth_parameter: 130       // Max stat for 4th jobs

// ASPD
max_aspd: 190
max_third_aspd: 193
max_fourth_aspd: 193

// Walk speed
default_walk_speed: 150
max_walk_speed: 100             // Lower = faster

// Weight
max_weight_base: 20000
max_cart_weight: 8000

// Skill points
skillup_limit: 0                // 0=no limit
```

**Common Changes:**
```conf
// High-rate server
max_parameter: 120
max_aspd: 196
hp_rate: 150
sp_rate: 150
```

---

## 4. MONSTER MECHANICS

### monster.conf

```conf
// Monster HP/damage rates
mob_count_rate: 100             // Spawn count
mob_hp_rate: 100                // HP multiplier
mob_max_casttime: 10000         // Max cast time (ms)

// MVP settings
mvp_tomb_enabled: yes           // Show MVP tombstone
mvp_exp_bonus_max_rate: 200    // Max MVP exp bonus
override_mob_names: no          // Use custom mob names

// Monster behavior
monster_active_enable: yes      // Aggressive monsters
mob_skill_rate: 100             // Skill usage rate
mob_skill_delay: 100            // Skill delay
mob_damage_delay: 100           // Damage display delay

// Monster info display
show_mob_info: 0                // 0=none, 1=name, 2=HP, 3=both
monster_hp_bars_info: yes       // Show HP bar
```

**Boss Enhancement:**
```conf
// Harder MVPs
mob_hp_rate: 150
mvp_exp_bonus_max_rate: 300
```

---

## 5. PARTY & GUILD

### party.conf

```conf
// Party exp sharing
party_even_share_bonus: 0       // Bonus exp % for even share
party_item_share_type: 0        // 0=each, 1=shared
party_hp_mode: 0                // 0=off, 1=show to party
display_party_name: no          // Show party name above player

// Party creation
party_update_interval: 1000     // HP update interval (ms)
```

### guild.conf

```conf
// Guild settings
guild_max_castles: 0            // Max castles per guild (0=unlimited)
guild_skill_relog_delay: 300000 // Guild skill delay after login
guild_exp_limit: 50             // Max guild level

// Guild storage
guild_storage_log: no           // Log guild storage
```

**Common Changes:**
```conf
// Show party HP
party_hp_mode: 1
display_party_name: yes

// Higher guild level
guild_exp_limit: 99
```

---

## 6. PVP & WOE

### battle.conf & gvg.conf

```conf
// PvP settings
pk_mode: 0                      // 0=off, 1=on, 2=nightwatch
pk_level_range: 0               // Level range for PvP (0=any)

// Damage adjustments
pk_short_attack_damage_rate: 100
pk_long_attack_damage_rate: 100
pk_magic_attack_damage_rate: 100
pk_misc_attack_damage_rate: 100

// WoE settings
castle_defense_rate: 100
gvg_short_attack_damage_rate: 80
gvg_long_attack_damage_rate: 80
gvg_magic_attack_damage_rate: 60
gvg_misc_attack_damage_rate: 80
gvg_flee_penalty: 20

// Guild vs Guild
gvg_eliminate_time: 7000        // Respawn time in GvG (ms)
```

**Balanced PvP:**
```conf
// Reduce burst damage
pk_short_attack_damage_rate: 80
pk_long_attack_damage_rate: 80
pk_magic_attack_damage_rate: 70
```

**Harder WoE:**
```conf
// Increase castle defense
castle_defense_rate: 150
gvg_flee_penalty: 30
```

---

## 7. ITEMS & EQUIPMENT

### items.conf

```conf
// Item settings
item_auto_get: no               // Auto-loot own drops
item_first_get_time: 3000       // First looter time (ms)
item_check_equip_weight: no     // Check weight when equipping
item_zeny_from_mobs: yes        // Monsters drop zeny

// Equipment breaking
equipment_breaking: no          // Can equipment break?
equipment_break_rate: 100       // Break rate

// Item usage
item_use_interval: 0            // Delay between item use (ms)
cashfood_use_box: no            // Cash food from boxes

// Vending
vending_over_max: yes           // Allow vending over max price
vending_tax: 200                // Tax on vending (200 = 2%)
vending_max_value: 1000000000   // Max price per item

// Production
weapon_produce_rate: 100        // Weapon craft success rate
potion_produce_rate: 100        // Potion craft success rate
```

**Common Changes:**
```conf
// Casual server
item_auto_get: yes
equipment_breaking: no

// Higher craft rates
weapon_produce_rate: 150
potion_produce_rate: 150
```

---

## 8. SKILLS

### skill.conf

```conf
// Skill settings
skill_delay_attack_enable: yes  // Can't attack while casting
castrate_dex_scale: 150         // DEX affect on cast time
vcast_stat_scale: 530           // Stat affect on variable cast

// Skill damage
skill_min_damage: 6             // Minimum skill damage

// Skill cool downs
skill_add_heal_rate: 7          // Heal bonus per VIT/INT

// Restrictions
gm_all_skill: no                // GMs get all skills
player_skill_partner_check: yes // Check partner for couple skills

// Casting
casting_rate: 100               // Cast time rate
delay_rate: 100                 // Delay rate
skill_delay_attack_enable: yes  // Delay attack during cast
```

**Instant Cast Server:**
```conf
// No cast time
casting_rate: 0
delay_rate: 50
```

---

## 9. DEATH PENALTIES

### exp.conf

```conf
// Death penalties
death_penalty_type: 1           // 0=none, 1=exp loss, 2=drop
death_penalty_base: 100         // Base exp loss (100 = 1%)
death_penalty_job: 100          // Job exp loss (100 = 1%)
zeny_penalty: 0                 // Zeny loss (100 = 1%)

// Resurrection exp loss
resurrection_exp: 0             // Exp loss on ress (0-100)

// PvP death penalty
death_penalty_maxlv: 0          // 0=always, 1=only below max level

// Exp from PvP
pk_exp: no                      // Gain exp from PvP kills
```

**No Penalty Server:**
```conf
death_penalty_type: 0
death_penalty_base: 0
death_penalty_job: 0
zeny_penalty: 0
```

**Hardcore Server:**
```conf
death_penalty_type: 1
death_penalty_base: 500         // 5% loss
death_penalty_job: 500
zeny_penalty: 200               // 2% zeny loss
```

---

## 10. MISC SETTINGS

### misc.conf

```conf
// Timers
day_duration: 7200000           // Day length (ms)
night_duration: 1800000         // Night length (ms)

// Chat
global_chat: yes                // Enable global chat
show_steal_in_same_party: no    // Show steal to party
map_local_channel_autojoin: no  // Auto-join map channel

// Commands
atcommand_spawn_quantity_limit: 100
atcommand_slave_clone_limit: 25
partial_name_scan: yes          // Search by partial name

// Display
display_status_timers: yes      // Show buff timers
display_hallucination: yes      // Hallucination effect
display_delay_skill_fail: yes   // Show skill fail message

// Autotrade
at_mapflag: no                  // Autotrade needs mapflag
at_timeout: 0                   // Autotrade timeout (minutes, 0=no limit)

// Mail
mail_show_status: 0             // 0=no, 1=yes, 2=if unread

// Banking
feature_banking: yes            // Enable banking system
```

**Common Changes:**
```conf
// 24/7 daylight
day_duration: 0
night_duration: 0

// Better QoL
display_status_timers: yes
mail_show_status: 2
at_mapflag: no
```

---

## CONFIGURATION TEMPLATE

### Recommended `/conf/import/battle_conf.txt`

```conf
//=============================================================
// EXPERIENCE RATES
//=============================================================
base_exp_rate: 1000             // 10x
job_exp_rate: 1000              // 10x
mvp_exp_rate: 1000              // 10x
quest_exp_rate: 1000            // 10x

//=============================================================
// DROP RATES
//=============================================================
item_rate_common: 200           // 2x
item_rate_heal: 200             // 2x
item_rate_use: 200              // 2x
item_rate_equip: 200            // 2x
item_rate_card: 100             // 1x (keep cards rare)

//=============================================================
// PLAYER SETTINGS
//=============================================================
max_parameter: 120
max_aspd: 196
hp_rate: 150
sp_rate: 150

//=============================================================
// DEATH PENALTY
//=============================================================
death_penalty_type: 0           // No penalty
death_penalty_base: 0
death_penalty_job: 0
zeny_penalty: 0

//=============================================================
// QUALITY OF LIFE
//=============================================================
item_auto_get: no
equipment_breaking: no
day_duration: 0
night_duration: 0
display_status_timers: yes
mail_show_status: 2

//=============================================================
// PARTY & GUILD
//=============================================================
party_hp_mode: 1
display_party_name: yes
guild_exp_limit: 99

//=============================================================
// SKILLS
//=============================================================
skill_delay_attack_enable: yes
castrate_dex_scale: 150

//=============================================================
// PRODUCTION
//=============================================================
weapon_produce_rate: 150
potion_produce_rate: 150
```

---

## RATE PRESETS

### Low Rate (Official-like)
```conf
base_exp_rate: 100
job_exp_rate: 100
item_rate_common: 100
item_rate_card: 100
death_penalty_type: 1
death_penalty_base: 100
```

### Mid Rate (5x)
```conf
base_exp_rate: 500
job_exp_rate: 500
item_rate_common: 200
item_rate_card: 100
death_penalty_type: 1
death_penalty_base: 50
```

### High Rate (10-50x)
```conf
base_exp_rate: 1000             // 10x
job_exp_rate: 1000
item_rate_common: 300
item_rate_card: 150
death_penalty_type: 0
```

### Super High Rate (100x+)
```conf
base_exp_rate: 10000            // 100x
job_exp_rate: 10000
item_rate_common: 1000          // 10x
item_rate_card: 500             // 5x
max_parameter: 150
death_penalty_type: 0
```

---

## APPLYING CHANGES

### Method 1: Restart Server (Recommended)
```bash
./map-server
```

### Method 2: Reload Config (In-Game)
```
@reloadbattleconf
```

**Note:** Some settings require full restart to take effect.

---

## TESTING RATES

```
// Check current rates
@rates

// Check your stats
@stats

// Test exp gain
@blvl 1          // Level up
@jlvl 1          // Job level up

// Test drops
@monster PORING 1
// Kill and check drops
```

---

## TIPS & BEST PRACTICES

1. **Start conservative** - Can always increase rates later
2. **Test thoroughly** - Changes affect game balance significantly
3. **Document changes** - Comment your battle_conf.txt
4. **Backup original** - Keep defaults for reference
5. **Consider players** - What experience do they want?
6. **Balance PvP** - Lower damage rates for better PvP
7. **Adjust together** - Exp and drops should match server style
8. **Monitor feedback** - Players will tell you if rates are wrong

---

## COMMON ISSUES

### Too Fast Leveling
```conf
// Reduce exp rates
base_exp_rate: 500  // Lower from 1000
```

### Economy Inflation
```conf
// Reduce drop rates
item_rate_common: 150  // Lower from 300
// Enable zeny sinks
vending_tax: 500       // 5% tax
```

### PvP Too Bursty
```conf
// Reduce damage
pk_short_attack_damage_rate: 70
pk_magic_attack_damage_rate: 60
```

---

## SEE ALSO

- **Configuration Files:** `/conf/battle/*.conf`
- **KB_REF_004:** Configuration System Guide
- **KB_CONF_001:** Server Configuration Overview

---

*Last Updated: 2025-10-23*
*rAthena Documentation - Battle Configuration*
