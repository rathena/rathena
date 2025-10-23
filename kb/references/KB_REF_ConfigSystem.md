---
kb_id: KB_REF_004
kb_type: reference
kb_category: server_configuration
kb_subcategory: import_system
kb_keywords: [configuration, import, override, conf, database, customization, import directory, server config, battle config, db override, git conflicts, update-safe]
kb_related: [KB_CONF_001, KB_DB_001]
kb_difficulty: basic
kb_version: rAthena_2025
kb_last_updated: 2025-10-23
kb_use_case: [server_setup, customization, maintenance, version_updates]
---

# rAthena Configuration & Database Import System

Complete guide to the import directory system for configuration and database customization.

## Overview

The **import directory** system allows you to customize your rAthena server WITHOUT modifying core files. This prevents merge conflicts during updates and keeps your customizations separate from base files.

**Core Concept:** Think of "import" as "override"

**Benefits:**
✅ Update-safe - No merge conflicts when updating rAthena
✅ Clean separation - Your changes vs. official code
✅ Easy backup - Just backup `/conf/import/` and `/db/import/`
✅ Maintainable - See exactly what you've customized
✅ Reversible - Delete import files to restore defaults

---

## HOW IT WORKS

### Directory Structure

```
rathena/
├── conf/                      # Core configuration files (DON'T EDIT)
│   ├── battle/
│   ├── import/                # YOUR customizations go here ✓
│   └── import-tmpl/           # Examples/templates
├── db/                        # Core database files (DON'T EDIT)
│   ├── (pre-)re/
│   ├── import/                # YOUR customizations go here ✓
│   └── import-tmpl/           # Examples/templates
└── npc/
    ├── scripts.conf
    └── custom/                # YOUR custom NPCs go here ✓
```

### Loading Order

1. **Core files** load first (e.g., `/conf/char_athena.conf`)
2. **Import files** load second (e.g., `/conf/import/char_conf.txt`)
3. Import files **override** core file settings

**Example:**
```
/conf/char_athena.conf:        server_name: rAthena
/conf/import/char_conf.txt:    server_name: MyServer

Result: server_name = "MyServer"
```

---

## CONFIGURATION FILES (`/conf/import/`)

### General Rules

1. **Only include settings you want to override**
2. **Use exact same setting names** as core files
3. **Comments are optional** but recommended for documentation
4. **One file per server** (login_conf.txt, char_conf.txt, map_conf.txt, etc.)

---

### Login Server Configuration

**File:** `/conf/import/login_conf.txt`

**Example:** Use MD5 passwords and disable account creation

```conf
// Disable _m/f account creation
new_account: no

// Use MD5 password hashing
use_MD5_passwords: yes

// Server port (if different from default)
login_port: 6900
```

**Common Settings:**
- `new_account` - Allow new account creation
- `use_MD5_passwords` - Password hashing method
- `login_port` - Server port
- `min_level_to_connect` - Minimum level requirement
- `check_client_version` - Client version enforcement

---

### Character Server Configuration

**File:** `/conf/import/char_conf.txt`

**Example:** Change server name and settings

```conf
// Server name shown in character selection
server_name: Odin

// Allow character deletion
char_del_option: 1

// Starting zeny amount
start_zeny: 10000

// Max characters per account
max_char_num: 9
```

**Common Settings:**
- `server_name` - Server display name
- `char_del_option` - Deletion options (0=block, 1=email, 2=birthdate)
- `start_zeny` - Starting money
- `start_point` - Starting location (map,x,y)
- `max_char_num` - Characters per account limit

---

### Map Server Configuration

**File:** `/conf/import/map_conf.txt`

**Example:** Hide errors and add custom maps

```conf
// Hide error messages (16 = hide Error and SQL Error)
console_silent: 16

// Add custom maps
map: 1@toy
map: 1@valley
map: shops
map: custom_pvp
map: custom_woe

// Server MOTD
motd_txt: conf/import/motd.txt

// Help message file
help_txt: conf/import/help.txt
```

**Common Settings:**
- `console_silent` - Output filtering
- `map` - Load additional maps
- `motd_txt` - Message of the day file
- `help_txt` - Help text file
- `autosave_time` - Auto-save interval
- `map_cache_file` - Map cache path

---

### Inter Server Configuration

**File:** `/conf/import/inter_conf.txt`

**Example:** Use SQL instead of TXT databases

```conf
// Use SQL database (recommended)
use_sql_db: yes

// MySQL connection settings
sql.db_hostname: 127.0.0.1
sql.db_port: 3306
sql.db_username: ragnarok
sql.db_password: ragnarok
sql.db_database: ragnarok

// Character server SQL settings
char_server_db: char
```

**Common Settings:**
- `use_sql_db` - Enable SQL storage
- `sql.db_hostname` - Database host
- `sql.db_port` - Database port
- `sql.db_username` - Database user
- `sql.db_password` - Database password
- `sql.db_database` - Database name

---

### Logging Configuration

**File:** `/conf/import/log_conf.txt`

**Example:** Log all items and chat messages

```conf
// Log filter (1 = log all items)
log_filter: 1

// Log all chat types (63 = all)
// 1=Global, 2=Whisper, 4=Party, 8=Guild, 16=Main, 32=Clan
log_chat: 63

// Log trades
log_trade: 1

// Log vending
log_vending: 1

// Log commands
log_commands: 1

// Log NPC transactions
log_npc: 1
```

**Common Settings:**
- `log_filter` - Item logging (1=all, 2=only valuables)
- `log_chat` - Chat logging (bitmask)
- `log_trade` - Trade logging
- `log_vending` - Vending logging
- `log_commands` - Command logging
- `log_npc` - NPC transaction logging

---

### Battle Configuration

**File:** `/conf/import/battle_conf.txt`

This is the **most important** import file. All battle mechanic changes go here.

**Example:** Customize various game mechanics

```conf
//=============================================================
// GUILD SETTINGS (from guild.conf)
//=============================================================

// Guild max level
guild_exp_limit: 90

// Guild skill cooldown (ms)
guild_skill_relog_delay: 0


//=============================================================
// ITEM SETTINGS (from items.conf)
//=============================================================

// Allow vending over max price
vending_over_max: no

// Vending tax (100 = 1%)
vending_tax: 100

// Weapon production success rate (200 = 2x)
weapon_produce_rate: 200

// Potion production success rate
potion_produce_rate: 200

// Item name input for production
produce_item_name_input: 0x03

// Max item stack
stack_amount_item: 30000
stack_amount_equip: 100


//=============================================================
// MISC SETTINGS (from misc.conf)
//=============================================================

// Duel time interval (minutes)
duel_time_interval: 2

// @autotrade requires mapflag
at_mapflag: yes

// @monsterignore behavior
at_monsterignore: yes

// Cash shop show points
cashshop_show_points: yes

// Hide favorites in sell window
hide_fav_sell: yes

// Mail box status on login (0=no, 1=yes, 2=yes if unread)
mail_show_status: 2


//=============================================================
// MONSTER SETTINGS (from monster.conf)
//=============================================================

// Show monster info (3 = name + HP)
// 0 = None, 1 = Name, 2 = HP, 3 = Both
show_mob_info: 3

// Monster HP bar info
monster_hp_bars_info: yes

// MVP tombstone time (ms)
mvp_tomb_enabled: yes


//=============================================================
// PARTY SETTINGS (from party.conf)
//=============================================================

// Party HP mode (1 = show to party)
party_hp_mode: 1

// Display party name
display_party_name: yes

// Party item share type
party_item_share_type: 1


//=============================================================
// PET SETTINGS (from pet.conf)
//=============================================================

// Allow pet renaming
pet_rename: yes

// Pet attack support
pet_attack_support: yes

// Pet damage support
pet_damage_support: yes


//=============================================================
// PLAYER SETTINGS (from player.conf)
//=============================================================

// Max ASPD
max_aspd: 196
max_third_aspd: 196
max_extended_aspd: 196

// Display VIP rates
vip_disp_rate: no

// Max stats
max_parameter: 99

// Max baby stats
max_baby_parameter: 80

// Natural heal weight rate (50 = 50% weight)
natural_heal_weight_rate: 50

// Zeny penalty on death
zeny_penalty: 0


//=============================================================
// EXP SETTINGS (from exp.conf)
//=============================================================

// Base exp rate (100 = 1x, 1000 = 10x)
base_exp_rate: 100

// Job exp rate
job_exp_rate: 100

// MVP exp rate
mvp_exp_rate: 100

// Quest exp rate
quest_exp_rate: 100


//=============================================================
// DROP SETTINGS (from drops.conf)
//=============================================================

// Item drop rate (100 = 1x, 200 = 2x)
item_rate_common: 100
item_rate_heal: 100
item_rate_use: 100
item_rate_equip: 100
item_rate_card: 100
item_rate_mvp: 100


//=============================================================
// SKILL SETTINGS (from skill.conf)
//=============================================================

// Skill delay attack (yes = can't attack while casting)
skill_delay_attack_enable: yes

// Cast cancel on damage
casting_rate: 100

// Skill failure rate
skill_fail_rate: 100


//=============================================================
// STATUS SETTINGS (from status.conf)
//=============================================================

// Debuff on logout (3 = remove buffs and debuffs)
// 0 = None, 1 = Buffs, 2 = Debuffs, 3 = Both
debuff_on_logout: 3

// Max walk speed
max_walk_speed: 150

// Status point cost for stats
// Formula: (x-2)^2 + (x-2)
// standard = 1, advanced = 2
// You can set this to 0 to disable stat point cost
status_point_cost: 1


//=============================================================
// HOMUNCULUS SETTINGS (from homunc.conf)
//=============================================================

// Homunculus autoloot
homunculus_autoloot: no

// Homunculus friendly rate (how often they help)
homunculus_friendly_rate: 100
```

**Battle Config Files Reference:**
- `battle.conf` - General battle mechanics
- `player.conf` - Player settings
- `monster.conf` - Monster behavior
- `skill.conf` - Skill mechanics
- `items.conf` - Item mechanics
- `exp.conf` - Experience rates
- `drops.conf` - Drop rates
- `party.conf` - Party settings
- `guild.conf` - Guild settings
- `pet.conf` - Pet system
- `homunc.conf` - Homunculus system
- `status.conf` - Status changes
- `feature.conf` - Feature toggles
- `client.conf` - Client settings
- `gm.conf` - GM settings
- `battleground.conf` - BG settings
- `instance.conf` - Instance settings

---

## DATABASE FILES (`/db/import/`)

### General Rules

1. **Use YAML format** for database files
2. **Follow exact structure** from main database files
3. **Custom IDs:** Use high ID numbers (30000+) to avoid conflicts
4. **Document your entries** with comments

---

### Custom Achievements

**File:** `/db/import/achievement_db.yml`

**Example:**

```yaml
# Custom Achievements
# Use IDs 280000+ for custom achievements

- Id: 280000
  Group: None
  Name: Emperio
  Reward:
    TitleId: 1035
  Score: 50

- Id: 280001
  Group: None
  Name: Staff
  Reward:
    TitleId: 1036
    ItemId: 607
    Amount: 10
  Score: 50
```

**Fields:**
- `Id` - Unique achievement ID (280000+)
- `Group` - Achievement category
- `Name` - Display name
- `Reward` - Title, items, etc.
- `Score` - Achievement points

---

### Custom Instances

**File:** `/db/import/instance_db.yml`

**Example:**

```yaml
# Custom Housing Instance

- Id: 35
  Name: Home
  TimeLimit: 7200         # 2 hours in seconds
  IdleTimeOut: 900        # 15 minutes idle timeout
  Enter:
    Map: 1@home
    X: 24
    Y: 6
  AdditionalMaps:
    - Map: 2@home
    - Map: 3@home
```

**Fields:**
- `Id` - Unique instance ID
- `Name` - Instance name
- `TimeLimit` - Max duration (seconds)
- `IdleTimeOut` - Idle kick time (seconds)
- `Enter` - Entry point
- `AdditionalMaps` - Extra maps in instance

---

### Monster Appearance Override (Mob Alias)

**File:** `/db/import/mob_avail.yml`

**Example:** Make Porings look like Baphomet

```yaml
# Mob Sprite Aliases

- Mob: PORING
  Sprite: BAPHOMET

- Mob: DROPS
  Sprite: EDDGA

- Mob: MARIN
  Sprite: OSIRIS
```

**Use Case:** Events, fun transformations, placeholder sprites.

---

### Custom Maps

**File:** `/db/import/map_index.txt`

**Example:**

```
// Custom Maps
// Format: <map_name>  {<map_index>}
// If map_index not specified, auto-assigned

1@home    1250
2@home    1251
3@home    1252
ev_has
shops
prt_pvp
custom_woe
```

**Notes:**
- Don't forget to add maps to map_cache.dat using mapcache tool
- Also add maps to `/conf/import/map_conf.txt`

---

### Custom Items

**File:** `/db/import/item_db.yml`

**Example:** Custom items with trade restrictions

```yaml
# Custom Items
# Use IDs 30000+ for custom items

- Id: 34000
  AegisName: Old_Green_Box
  Name: Old Green Box
  Type: Usable
  Buy: 100000
  Weight: 200
  Script: |
    getrandgroupitem 34000,1;
  Trade:
    NoDrop: true
    NoTrade: true
    TradePartner: true
    NoSell: true
    NoCart: true
    NoStorage: true
    NoGuildStorage: true
    NoMail: true
    NoAuction: true

- Id: 34001
  AegisName: House_Keys
  Name: House Keys
  Type: Etc
  Buy: 0
  Sell: 0
  Weight: 10
  Trade:
    NoDrop: true
    NoTrade: true
    TradePartner: true
    NoSell: true
    NoCart: true
    NoStorage: true
    NoGuildStorage: true
    NoMail: true
    NoAuction: true

- Id: 34002
  AegisName: Reputation_Journal
  Name: Reputation Journal
  Type: Etc
  Buy: 0
  Sell: 0
  Weight: 50
  Script: |
    mes "Your reputation: " + #REPUTATION;
    close;
  Trade:
    NoDrop: true
    NoTrade: true
    TradePartner: false
    NoSell: true
    NoCart: true
    NoStorage: true
    NoGuildStorage: true
    NoMail: true
    NoAuction: true
```

**Trade Restrictions:**
- `NoDrop` - Can't drop
- `NoTrade` - Can't trade
- `TradePartner` - Can trade to partner only
- `NoSell` - Can't sell to NPC
- `NoCart` - Can't put in cart
- `NoStorage` - Can't put in Kafra storage
- `NoGuildStorage` - Can't put in guild storage
- `NoMail` - Can't send via mail
- `NoAuction` - Can't auction

---

### Custom Monsters

**File:** `/db/import/mob_db.yml`

**Example:**

```yaml
# Custom Monsters
# Use IDs 30000+ for custom monsters

- Id: 30000
  AegisName: CUSTOM_PORING
  Name: Custom Poring
  Level: 99
  Hp: 1000000
  Sp: 0
  BaseExp: 50000
  JobExp: 30000
  Attack: 1000
  Attack2: 1500
  Defense: 50
  MagicDefense: 40
  Str: 50
  Agi: 50
  Vit: 50
  Int: 50
  Dex: 50
  Luk: 50
  AttackRange: 1
  SkillRange: 10
  ChaseRange: 12
  Size: Small
  Race: Plant
  Element: Water
  ElementLevel: 1
  WalkSpeed: 400
  AttackDelay: 1872
  AttackMotion: 672
  DamageMotion: 480
  Drops:
    - Item: Jellopy
      Rate: 10000
    - Item: Knife
      Rate: 1000
    - Item: Red_Potion
      Rate: 5000
```

---

### Custom Quests

**File:** `/db/import/quest_db.yml`

**Example:**

```yaml
# Custom Quests
# Use IDs 89000+ for custom quests

- Id: 89001
  Title: "Reputation Quest"
  Targets:
    - Mob: PORING
      Count: 100
  Drops:
    - Mob: PORING
      Item: Jellopy
      Count: 1
      Rate: 5000

- Id: 89002
  Title: "Weekly Boss Hunt"
  TimeLimit: Monday 4h
  Targets:
    - Mob: BAPHOMET
      Count: 1
    - Mob: EDDGA
      Count: 1
  Drops:
    - Mob: BAPHOMET
      Item: Evil_Horn
      Count: 1
      Rate: 10000
```

---

## CUSTOM NPC SCRIPTS

**Location:** `/npc/custom/`

**Loading:** Add to `/npc/scripts_custom.conf` or create in `/npc/custom/` directory

**Example:** `/npc/custom/my_custom_npc.txt`

```c
//===== rAthena Script =======================================
//= My Custom NPC
//===== By: ==================================================
//= YourName
//===== Description: =========================================
//= Custom functionality for my server
//============================================================

prontera,150,150,4	script	Custom NPC	4_F_KAFRA1,{
    mes "[Custom NPC]";
    mes "Welcome to my custom server!";
    next;
    mes "[Custom NPC]";
    mes "What would you like?";
    next;
    switch(select("Buff Me:Heal Me:Information:Cancel")) {
    case 1:
        sc_start SC_BLESSING,240000,10;
        sc_start SC_INCREASEAGI,240000,10;
        mes "Buffed!";
        close;
    case 2:
        percentheal 100,100;
        mes "Healed!";
        close;
    case 3:
        mes "Server Info Here";
        close;
    case 4:
        close;
    }
}
```

**Load in** `/npc/scripts_custom.conf`:
```
npc: npc/custom/my_custom_npc.txt
```

---

## BEST PRACTICES

### ✅ DO

1. **Always use import directories** for customizations
2. **Document your changes** with comments
3. **Use high IDs** for custom content (30000+ items, 30000+ mobs, 89000+ quests, 280000+ achievements)
4. **Backup import directories** regularly
5. **Test changes** in development environment first
6. **Keep imports organized** by category
7. **Version control** your import directory
8. **Use consistent naming** conventions

### ❌ DON'T

1. **Don't edit core files** in `/conf/` or `/db/` directly
2. **Don't use conflicting IDs** with official content
3. **Don't forget** to reload server after changes
4. **Don't copy entire files** to import - only what you change
5. **Don't forget syntax** - YAML is whitespace-sensitive
6. **Don't skip backups** before major changes

---

## UPDATING RATHENA

### Safe Update Workflow

```bash
# 1. Backup your import directories
cp -r conf/import conf/import.backup
cp -r db/import db/import.backup
cp -r npc/custom npc/custom.backup

# 2. Pull latest rAthena updates
git pull origin master

# 3. Check for conflicts (there should be none if using imports)
git status

# 4. Recompile server
./configure && make clean && make server

# 5. Test your changes
# Start server and verify everything works

# 6. If issues occur, restore from backup
# cp -r conf/import.backup/* conf/import/
```

**Why This Works:**
- Core files get updated
- Your import files remain untouched
- No merge conflicts
- Easy rollback if needed

---

## TROUBLESHOOTING

### Issue: Changes not applying

**Solutions:**
1. Check file is in correct `/import/` directory
2. Verify syntax (YAML is picky about indentation)
3. Reload server/scripts: `@reloadscript`
4. Check for typos in setting names
5. Ensure import file has correct extension (`.txt` for conf, `.yml` for db)

### Issue: Server won't start

**Solutions:**
1. Check syntax errors in import files
2. Review server console output for errors
3. Temporarily rename import file to disable it
4. Test with minimal imports first

### Issue: Database entries not loading

**Solutions:**
1. Verify YAML formatting (spaces, not tabs)
2. Check ID conflicts with official content
3. Ensure file is named correctly
4. Check that imports are enabled in main files

---

## IMPORT FILE LOCATIONS REFERENCE

### Configuration Files
```
/conf/import/
├── login_conf.txt           # Login server settings
├── char_conf.txt            # Char server settings
├── map_conf.txt             # Map server settings
├── inter_conf.txt           # Inter-server settings
├── log_conf.txt             # Logging settings
├── battle_conf.txt          # Battle mechanics (IMPORTANT)
├── script_conf.txt          # Script settings
├── packet_conf.txt          # Packet settings
└── channels_conf.txt        # Chat channels
```

### Database Files
```
/db/import/
├── achievement_db.yml       # Custom achievements
├── instance_db.yml          # Custom instances
├── item_db.yml              # Custom items
├── mob_db.yml               # Custom monsters
├── quest_db.yml             # Custom quests
├── skill_db.yml             # Custom skills
├── mob_avail.yml            # Mob sprite aliases
├── map_index.txt            # Custom map list
└── ... (any database file can be imported)
```

---

## EXAMPLE: COMPLETE SERVER SETUP

Here's a typical server customization setup:

### `/conf/import/char_conf.txt`
```conf
server_name: MyServer
start_zeny: 50000
start_point: prontera,156,191
```

### `/conf/import/battle_conf.txt`
```conf
base_exp_rate: 500
job_exp_rate: 500
item_rate_common: 200
max_aspd: 196
```

### `/db/import/item_db.yml`
```yaml
- Id: 34000
  AegisName: Starter_Box
  Name: Starter Box
  Type: Usable
  Buy: 0
  Weight: 0
  Script: |
    getitem 501,100;  // Red Potion
    getitem 503,50;   // Yellow Potion
    getitem 601,10;   // Fly Wing
```

### `/npc/custom/starter_npc.txt`
```c
prontera,156,185,4	script	Starter Helper	4_F_KAFRA1,{
    if (#STARTER_RECEIVED) {
        mes "Welcome back!";
        close;
    }
    mes "Welcome! Here's a starter box!";
    getitem 34000,1;
    set #STARTER_RECEIVED,1;
    close;
}
```

---

## SUMMARY

**Import System = Update-Safe Customization**

- 🔒 **Safe:** Never touch core files
- 🔄 **Update-Friendly:** No merge conflicts
- 🎯 **Organized:** Clear separation of custom vs official
- 💾 **Backup-Friendly:** Just backup import directories
- 🔧 **Maintainable:** See exactly what you changed

**Remember:** Only override what you need to change!

---

## SEE ALSO

- **KB_CONF_001:** Battle Configuration Deep Dive
- **KB_DB_001:** Database System Overview
- **KB_EXAMPLE_001:** Configuration Examples

---

*Last Updated: 2025-10-23*
*rAthena Documentation*
