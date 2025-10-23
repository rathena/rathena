---
kb_id: KB_REF_002
kb_type: reference
kb_category: database
kb_subcategory: quest_system
kb_keywords: [quest, quest_db, YAML, objectives, targets, drops, rewards, TimeLimit, mob kill, quest system, quest database]
kb_related: [KB_DB_001, KB_EXAMPLE_005]
kb_difficulty: intermediate
kb_version: rAthena_2025
kb_last_updated: 2022-06-29
kb_use_case: [quest_creation, content_creation, quest_scripting]
---

# rAthena Quest Database Structure

Complete reference for the quest database structure in `/db/(pre-)re/quest_db.yml`.

## Overview

The quest database defines all quests available in rAthena, including their objectives, time limits, drop rates, and rewards. Quests are configured using YAML format.

**Database Location:** `/db/(pre-)re/quest_db.yml`

---

## QUEST STRUCTURE

### Basic Quest Format

```yaml
- Id: 1000
  Title: Quest Name
  TimeLimit: <optional>
  Targets: <optional>
  Drops: <optional>
```

---

## FIELD REFERENCE

### Id (Required)
**Type:** Integer
**Description:** Unique quest identifier

**Example:**
```yaml
- Id: 1000
  Title: Poring Hunt
```

**Notes:**
- Must be unique across all quests
- Used in script commands like `questprogress()`, `setquest()`, `completequest()`
- Standard range: User quests typically start from 1000+

---

### Title (Required)
**Type:** String
**Description:** Display name of the quest shown to players

**Example:**
```yaml
- Id: 1000
  Title: Hunt 10 Porings
```

**Notes:**
- Shown in quest log UI
- Can contain spaces and special characters
- Keep concise for UI readability

---

### TimeLimit (Optional)
**Type:** String
**Description:** Quest expiration time or duration

Quest time limits can be specified in two ways:

#### **1. Relative Time Limit (Duration)**

Format: `+<time>` (starts when quest is taken)

**Syntax:** `+[d]d [h]h [mn]mn [s]s`
- `d` = days (optional)
- `h` = hours [0-23] (optional)
- `mn` = minutes [0-59] (optional)
- `s` = seconds [0-59] (optional)

**Examples:**
```yaml
# Quest expires 5 minutes after being taken
- Id: 2069
  Title: Tierra Gorge Battle
  TimeLimit: +5mn

# Quest expires 2 hours after being taken
- Id: 1001
  Title: Timed Challenge
  TimeLimit: +2h

# Quest expires 1 day and 30 minutes after being taken
- Id: 1002
  Title: Daily Quest Extended
  TimeLimit: +1d 30mn

# Quest expires 3 days, 12 hours, 30 minutes after being taken
- Id: 1003
  Title: Long Term Quest
  TimeLimit: +3d 12h 30mn
```

#### **2. Absolute Time Limit (Fixed Expiration)**

Format: `<date/day> <time>` (expires at specific time)

**Syntax (Option 1):** `<d>d [h]h [mn]mn [s]s`
**Syntax (Option 2):** `<DayOfWeek> [h]h [mn]mn [s]s`

**Examples:**
```yaml
# Quest expires 3 days from now at 4am
- Id: 9419
  Title: Attack Sky Fortress
  TimeLimit: 3d 4h

# Quest expires next Monday at 4am
- Id: 5965
  Title: "[Standby] Devil's Special"
  TimeLimit: Monday 4h

# Quest expires next Friday at 23:30
- Id: 1004
  Title: Weekly Challenge
  TimeLimit: Friday 23h 30mn

# Quest expires in 7 days at midnight
- Id: 1005
  Title: Week-Long Quest
  TimeLimit: 7d 0h
```

**Days of Week:** Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday

---

### Targets (Optional)
**Type:** Array
**Description:** Quest objectives - monsters to kill or conditions to meet

Targets can be defined in two ways:

#### **Method 1: Simple Mob Targeting**

Used for straightforward "kill X monsters" quests.

**Required Fields:**
- `Mob`: Monster name (AegisName from mob_db)
- `Count`: Number of monsters to kill

**Example:**
```yaml
Targets:
  - Mob: PORING
    Count: 10
  - Mob: DROPS
    Count: 5
```

**Notes:**
- `Count: 0` will skip the target on import (useful for disabled objectives)
- Mob must exist in `mob_db.yml`

---

#### **Method 2: Advanced Targeting (Race/Size/Element/Level)**

Used for flexible targeting by monster characteristics.

**Required Fields:**
- `Id`: Unique target index (positive number)
- `Count`: Number of kills required

**Optional Filters:**
- `Race`: Monster race filter
- `Size`: Monster size filter
- `Element`: Monster element filter
- `MinLevel`: Minimum monster level
- `MaxLevel`: Maximum monster level
- `Location`: Map where kills count
- `MapName`: Display name for location
- `MapMobTargets`: Specific monster whitelist/blacklist

**Example:**
```yaml
Targets:
  # Kill any 20 Demon-race monsters
  - Id: 1
    Count: 20
    Race: Demon

  # Kill 15 large monsters
  - Id: 2
    Count: 15
    Size: Large

  # Kill 10 Fire-element monsters between level 50-70
  - Id: 3
    Count: 10
    Element: Fire
    MinLevel: 50
    MaxLevel: 70

  # Kill 30 monsters on a specific map
  - Id: 4
    Count: 30
    Location: prontera
    MapName: Prontera
```

---

### Target Field Details

#### **Race**
**Valid Values:** `Angel`, `Brute`, `DemiHuman`, `Demon`, `Dragon`, `Fish`, `Formless`, `Insect`, `Plant`, `Undead`, `All`

**Default:** `All`

**Example:**
```yaml
Targets:
  - Id: 1
    Count: 25
    Race: Undead    # Only Undead monsters count
```

---

#### **Size**
**Valid Values:** `Small`, `Medium`, `Large`, `All`

**Default:** `All`

**Example:**
```yaml
Targets:
  - Id: 1
    Count: 15
    Size: Large     # Only Large monsters count
```

---

#### **Element**
**Valid Values:** `Dark`, `Earth`, `Fire`, `Ghost`, `Holy`, `Neutral`, `Poison`, `Undead`, `Water`, `Wind`, `All`

**Default:** `All`

**Example:**
```yaml
Targets:
  - Id: 1
    Count: 20
    Element: Fire   # Only Fire-element monsters count
```

---

#### **MinLevel / MaxLevel**
**Type:** Integer
**Default:**
- `MinLevel`: 1 (if MaxLevel defined)
- `MaxLevel`: No limit

**Notes:**
- Set to `0` to ignore the limit on import

**Example:**
```yaml
Targets:
  # Kill monsters level 30-50
  - Id: 1
    Count: 50
    MinLevel: 30
    MaxLevel: 50

  # Kill monsters level 80+
  - Id: 2
    Count: 20
    MinLevel: 80
```

---

#### **Location / MapName**
**Location Type:** String (map name without .gat)
**MapName Type:** String (display name)

**Example:**
```yaml
Targets:
  - Id: 1
    Count: 100
    Location: prontera
    MapName: Prontera City
```

**Notes:**
- Kills only count on the specified map
- `MapName` is shown in the quest UI

---

#### **MapMobTargets**
**Type:** Dictionary
**Description:** Whitelist/blacklist specific monsters by name

**Format:**
```yaml
MapMobTargets:
  <MonsterName>: <true/false>
```

- `true`: Add monster to whitelist
- `false`: Remove monster from whitelist

**Example:**
```yaml
Targets:
  - Id: 1
    Count: 50
    Location: prontera
    MapMobTargets:
      PORING: true       # Only Porings count
      DROPS: true        # Drops also count
      POPORING: true     # Poporings also count
```

**Notes:**
- Only active when using `Id` method (not `Mob` method)
- Allows precise control over which monsters count

---

## DROPS

**Type:** Array
**Description:** Quest-specific item drop configuration

When a quest is active, you can configure special drops from monsters.

**Fields:**
- `Mob`: Monster ID or name (0 = all monsters)
- `Item`: Item name (AegisName from item_db)
- `Count`: Number of items that drop
- `Rate`: Drop rate (10000 = 100%)

**Example:**
```yaml
Drops:
  # Drop Quest Item from specific monster
  - Mob: PORING
    Item: Jellopy
    Count: 1
    Rate: 5000        # 50% drop rate

  # Drop from any monster
  - Mob: 0
    Item: Quest_Token
    Count: 1
    Rate: 1000        # 10% drop rate

  # Drop multiple items at once
  - Mob: BOSS_MONSTER
    Item: Rare_Item
    Count: 3
    Rate: 10000       # 100% drop rate (3 items)
```

**Notes:**
- `Mob: 0` applies to ALL monsters
- `Count` defaults to 1 for non-stackable items
- `Rate` is in basis points (10000 = 100%, 5000 = 50%, 100 = 1%, 1 = 0.01%)
- Drops only occur while quest is active

---

## COMPLETE QUEST EXAMPLES

### Example 1: Simple Kill Quest
```yaml
- Id: 1000
  Title: Poring Extermination
  Targets:
    - Mob: PORING
      Count: 30
```

**Description:** Kill 30 Porings. No time limit.

---

### Example 2: Timed Quest with Drops
```yaml
- Id: 1001
  Title: Emergency Poring Alert
  TimeLimit: +1h
  Targets:
    - Mob: PORING
      Count: 50
  Drops:
    - Mob: PORING
      Item: Poring_Coin
      Count: 1
      Rate: 5000
```

**Description:** Kill 50 Porings within 1 hour. Porings have 50% chance to drop Poring Coin.

---

### Example 3: Multi-Target Quest
```yaml
- Id: 1002
  Title: Slime Cleanup
  Targets:
    - Mob: PORING
      Count: 20
    - Mob: DROPS
      Count: 15
    - Mob: POPORING
      Count: 10
```

**Description:** Kill 20 Porings, 15 Drops, and 10 Poporings.

---

### Example 4: Advanced Race/Element Quest
```yaml
- Id: 1003
  Title: Demon Hunter
  TimeLimit: +1d
  Targets:
    - Id: 1
      Count: 50
      Race: Demon
      MinLevel: 40
      MaxLevel: 80
```

**Description:** Kill 50 Demon-race monsters between level 40-80 within 24 hours.

---

### Example 5: Location-Specific Quest
```yaml
- Id: 1004
  Title: Prontera Patrol
  Targets:
    - Id: 1
      Count: 100
      Location: prt_fild08
      MapName: Prontera Field
```

**Description:** Kill 100 monsters in Prontera Field (prt_fild08).

---

### Example 6: Weekly Quest
```yaml
- Id: 1005
  Title: Weekly Challenge
  TimeLimit: Monday 4h
  Targets:
    - Id: 1
      Count: 200
      Race: Undead
  Drops:
    - Mob: 0
      Item: Weekly_Token
      Count: 1
      Rate: 2000
```

**Description:** Kill 200 Undead-race monsters before Monday 4am. All monsters have 20% chance to drop Weekly Token.

---

### Example 7: Boss Hunt Quest
```yaml
- Id: 1006
  Title: MVP Elimination
  Targets:
    - Mob: EDDGA
      Count: 1
    - Mob: OSIRIS
      Count: 1
    - Mob: BAPHOMET
      Count: 1
  Drops:
    - Mob: EDDGA
      Item: Eddga_Trophy
      Count: 1
      Rate: 10000
    - Mob: OSIRIS
      Item: Osiris_Trophy
      Count: 1
      Rate: 10000
    - Mob: BAPHOMET
      Item: Baphomet_Trophy
      Count: 1
      Rate: 10000
```

**Description:** Kill Eddga, Osiris, and Baphomet once each. Each drops a guaranteed trophy item.

---

## QUEST SCRIPT COMMANDS

Use these script commands to interact with quests:

### setquest(<quest_id>)
Gives the quest to the player.

```c
setquest(1000);  // Start quest 1000
```

---

### completequest(<quest_id>)
Marks the quest as completed.

```c
if (questprogress(1000, PLAYTIME) == 2) {
    completequest(1000);
    mes "Quest completed!";
}
```

---

### erasequest(<quest_id>)
Removes the quest from the player.

```c
erasequest(1000);  // Remove quest 1000
```

---

### questprogress(<quest_id>{, <type>})
Checks quest progress.

**Types:**
- `PLAYTIME` (0): Check if quest time expired
- `HUNTING` (1): Check if hunt objectives completed
- `HUNTING | PLAYTIME` (2): Check both

**Returns:**
- `0`: Quest not started
- `1`: Quest active
- `2`: Quest complete/expired

```c
if (questprogress(1000, HUNTING) == 2) {
    mes "You completed all hunt objectives!";
}
```

---

### checkquest(<quest_id>{, <type>})
Alias for `questprogress()`.

---

## QUEST STATUS VALUES

| Value | Constant | Meaning |
|-------|----------|---------|
| 0 | QUEST_NOT_STARTED | Quest not started |
| 1 | QUEST_ACTIVE | Quest active/in progress |
| 2 | QUEST_COMPLETE | Quest completed |

---

## BEST PRACTICES

1. **Unique IDs:** Always use unique quest IDs (avoid conflicts)
2. **Time Limits:** Use relative time (`+`) for recurring quests, absolute time for weekly/event quests
3. **Drop Rates:** Balance drop rates carefully (too high = no challenge, too low = frustration)
4. **Target Count:** Set `Count: 0` to temporarily disable objectives without deleting them
5. **Testing:** Test time limits thoroughly (server timezone matters!)
6. **Performance:** Avoid `Mob: 0` (all monsters) for drops when possible - use specific mobs
7. **UI Display:** Keep `Title` and `MapName` short for better UI display

---

## COMMON MISTAKES

### ❌ Wrong:
```yaml
- Id: 1000
  Title: Kill Quest
  TimeLimit: 5mn           # Missing +
  Targets:
    - Mob: INVALID_MOB     # Mob doesn't exist
      Count: -5            # Negative count
```

### ✓ Correct:
```yaml
- Id: 1000
  Title: Kill Quest
  TimeLimit: +5mn          # Relative time
  Targets:
    - Mob: PORING          # Valid mob from mob_db
      Count: 10            # Positive count
```

---

## RELATED FILES

- `/db/(pre-)re/quest_db.yml` - Quest database
- `/doc/script_commands.txt` - Quest-related script commands
- `/db/(pre-)re/mob_db.yml` - Monster names for Targets
- `/db/(pre-)re/item_db.yml` - Item names for Drops

---

## SEE ALSO

- **KB_EXAMPLE_005:** Quest Implementation Examples
- **KB_CMD_015:** Quest Script Commands
- **KB_DB_001:** Database System Overview

---

*Last Updated: 2022-06-29*
*rAthena Documentation*
