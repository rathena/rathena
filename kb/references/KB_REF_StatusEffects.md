---
kb_id: KB_REF_006
kb_type: reference
kb_category: game_mechanics
kb_subcategory: status_effects
kb_keywords: [status effects, status changes, SC_, buffs, debuffs, sc_start, sc_end, stone, freeze, stun, poison, blessing, increase agi, val1, val2, val3, val4]
kb_related: [KB_REF_005, KB_CMD_010]
kb_difficulty: intermediate
kb_version: rAthena_2025
kb_last_updated: 2024-10-24
kb_use_case: [npc_scripting, item_scripting, skill_effects, buff_management]
---

# rAthena Status Effects Reference

Complete reference for status effects (status changes) in rAthena.

## Overview

Status effects (also called "status changes" or "SC") are temporary conditions applied to characters that modify their stats, behavior, or appearance. They include buffs, debuffs, ailments, and special states.

**Script Usage:**
```c
// Apply status effect
sc_start <SC_CONST>,<duration_ms>,<val1>{,<rate>,<flag>,<unit_id>};

// Remove status effect
sc_end <SC_CONST>;

// Check if status effect is active
if (getstatus(SC_STONE)) {
    mes "You are petrified!";
}
```

**Duration:** Always in milliseconds (1000 = 1 second, 60000 = 1 minute)

---

## TABLE OF CONTENTS

1. [Basic Status Ailments](#1-basic-status-ailments)
2. [Stat Buffs](#2-stat-buffs)
3. [Stat Debuffs](#3-stat-debuffs)
4. [Element Changes](#4-element-changes)
5. [Combat Buffs](#5-combat-buffs)
6. [Support Effects](#6-support-effects)
7. [Special States](#7-special-states)
8. [Advanced Usage](#8-advanced-usage)

---

## 1. BASIC STATUS AILMENTS

### SC_STONE
**Name:** Stone / Petrify
**Effect:** DEF -50%; MDEF +25%; Element becomes Earth Lv1; Lose 1% HP/5sec (if HP>25%); Can't move/attack/use items/use skills

**Usage:**
```c
sc_start SC_STONE,30000,1;  // 30 seconds
```

**val1:** (not used)
**val2:** Caster's object ID
**val3:** Incubation time
**val4:** Remaining tick

---

### SC_FREEZE
**Name:** Frozen
**Effect:** DEF -50%; FLEE = 0; MDEF +25%; Element becomes Water Lv1; Can't move/attack/use items

**Usage:**
```c
sc_start SC_FREEZE,10000,1;  // 10 seconds
```

---

### SC_STUN
**Name:** Stunned
**Effect:** FLEE = 0; Can't move/attack/pick items/use items/use skills

**Usage:**
```c
sc_start SC_STUN,5000,1;  // 5 seconds
```

**Common Use:** Boss skills, PvP stunning

---

### SC_SLEEP
**Name:** Sleep
**Effect:** FLEE = 0; Enemy CRIT ×2; Can't move/attack/use items/use skills

**Usage:**
```c
sc_start SC_SLEEP,20000,1;  // 20 seconds
```

**Note:** Breaks when hit

---

### SC_POISON
**Name:** Poisoned
**Effect:** DEF -25%; Lose 1.5% + 2 HP/sec (if HP>25%); SP regen disabled

**Usage:**
```c
sc_start SC_POISON,60000,5;  // 60 seconds, level 5
```

**val1:** Skill level
**val2:** Caster's object ID
**val4:** Remaining tick

---

### SC_CURSE
**Name:** Cursed
**Effect:** ATK -25%; LUK = 0; Movement speed -300

**Usage:**
```c
sc_start SC_CURSE,30000,1;  // 30 seconds
```

**Note:** Can be removed with Blessing

---

### SC_SILENCE
**Name:** Silenced
**Effect:** Can't use active skills

**Usage:**
```c
sc_start SC_SILENCE,20000,1;  // 20 seconds
```

**Common Use:** Anti-caster debuff

---

### SC_BLIND
**Name:** Blinded
**Effect:** HIT -25%; FLEE -25%; Screen darkened

**Usage:**
```c
sc_start SC_BLIND,30000,1;  // 30 seconds
```

---

### SC_CONFUSION
**Name:** Confused
**Effect:** Move randomly; DEF = (STR + (INT×50))

**Usage:**
```c
sc_start SC_CONFUSION,15000,1;  // 15 seconds
```

---

### SC_BLEEDING
**Name:** Bleeding
**Effect:** HP regen disabled; SP regen disabled; Lose HP overtime

**Usage:**
```c
sc_start SC_BLEEDING,30000,5;  // 30 seconds, level 5
```

**val1:** Skill level
**val2:** Caster's object ID
**val4:** Remaining tick

---

### SC_DPOISON
**Name:** Deadly Poison
**Effect:** DEF -25%; Lose 10-15% HP/sec (if HP>25%)

**Usage:**
```c
sc_start SC_DPOISON,60000,10;  // 60 seconds, level 10
```

**Note:** More severe than SC_POISON

---

## 2. STAT BUFFS

### SC_BLESSING
**Name:** Blessing
**Effect:** Increase STR, DEX, INT by skill level; Removes Stone and Curse

**Usage:**
```c
sc_start SC_BLESSING,240000,10;  // 4 minutes, level 10 (+10 STR/DEX/INT)
```

**val1:** Skill level (stat bonus)

**Note:** If used on Undead/Demon mobs, reduces DEX/INT by 50%

---

### SC_INCREASEAGI
**Name:** Increase AGI
**Effect:** Increase AGI and movement speed

**Usage:**
```c
sc_start SC_INCREASEAGI,240000,10;  // 4 minutes, level 10
```

**val1:** Skill level

---

### SC_DECREASEAGI
**Name:** Decrease AGI
**Effect:** Decrease AGI and movement speed

**Usage:**
```c
sc_start SC_DECREASEAGI,40000,10;  // 40 seconds, level 10
```

**val1:** Skill level

---

### SC_GLORIA
**Name:** Gloria
**Effect:** LUK +30

**Usage:**
```c
sc_start SC_GLORIA,30000,1;  // 30 seconds
```

---

### SC_LOUD
**Name:** Loud Exclamation
**Effect:** STR +4

**Usage:**
```c
sc_start SC_LOUD,300000,1;  // 5 minutes
```

---

### SC_CONCENTRATE
**Name:** Attention Concentrate
**Effect:** AGI +(2+level)%; DEX +(2+level)%; Reveal hidden enemies in 3×3 area

**Usage:**
```c
sc_start SC_CONCENTRATE,60000,10;  // 60 seconds, level 10
```

---

## 3. STAT DEBUFFS

### SC_PROVOKE
**Name:** Provoke
**Effect:** DEF -(5+(5×level))%; ATK +(2+(3×level))%

**Usage:**
```c
sc_start SC_PROVOKE,30000,10;  // 30 seconds, level 10
```

**Note:** Lowers defense but increases attack

---

### SC_SIGNUMCRUCIS
**Name:** Signum Crucis
**Effect:** Decrease DEF of Undead and Demon mobs by (10+(4×level))%

**Usage:**
```c
sc_start SC_SIGNUMCRUCIS,40000,10;  // 40 seconds, level 10
```

---

### SC_QUAGMIRE
**Name:** Quagmire
**Effect:** Removes several AGI buffs; Movement speed -50%; AGI/DEX -(10×level)

**Usage:**
```c
sc_start SC_QUAGMIRE,20000,5;  // 20 seconds, level 5
```

**Removes:**
- Increase AGI
- Two-Hand Quicken
- Wind Walk
- Adrenaline Rush
- Attention Concentrate
- Cart Boost

---

## 4. ELEMENT CHANGES

### SC_ASPERSIO
**Name:** Aspersio
**Effect:** Change weapon element to HOLY

**Usage:**
```c
sc_start SC_ASPERSIO,180000,1;  // 3 minutes
```

---

### SC_BENEDICTIO
**Name:** B.S Sacramenti
**Effect:** Change armor element to HOLY

**Usage:**
```c
sc_start SC_BENEDICTIO,120000,1;  // 2 minutes
```

---

### SC_ENCPOISON
**Name:** Enchant Poison
**Effect:** Change weapon element to POISON; 2.5-3% poison chance

**Usage:**
```c
sc_start SC_ENCPOISON,180000,10;  // 3 minutes
```

---

## 5. COMBAT BUFFS

### SC_TWOHANDQUICKEN
**Name:** Two-Hand Quicken
**Effect:** ASPD +30%

**Usage:**
```c
sc_start SC_TWOHANDQUICKEN,300000,10;  // 5 minutes
```

---

### SC_ADRENALINE
**Name:** Adrenaline Rush
**Effect:** ASPD of Axe & Mace weapons ×2

**Usage:**
```c
sc_start SC_ADRENALINE,300000,5;  // 5 minutes
```

---

### SC_WEAPONPERFECTION
**Name:** Weapon Perfection
**Effect:** Ignore size penalty damage reduction

**Usage:**
```c
sc_start SC_WEAPONPERFECTION,40000,5;  // 40 seconds
```

**Note:** Small/Medium weapons deal full damage to Large monsters

---

### SC_OVERTHRUST
**Name:** Over Thrust
**Effect:** ATK +(5×level)%; 0.1% weapon break chance

**Usage:**
```c
sc_start SC_OVERTHRUST,300000,5;  // 5 minutes, level 5
```

**Note:** Axes, Maces, and Unbreakable weapons immune to breaking

---

### SC_MAXIMIZEPOWER
**Name:** Maximize Power
**Effect:** SP regen disabled; Damage always deals max damage

**Usage:**
```c
sc_start SC_MAXIMIZEPOWER,60000,1;  // 60 seconds
```

---

### SC_IMPOSITIO
**Name:** Impositio Manus
**Effect:** ATK +(5×level)

**Usage:**
```c
sc_start SC_IMPOSITIO,60000,5;  // 60 seconds, level 5 (+25 ATK)
```

---

### SC_AETERNA
**Name:** Lex Aeterna
**Effect:** Damage received ×2 (next hit only)

**Usage:**
```c
sc_start SC_AETERNA,600000,1;  // 10 minutes (until hit)
```

**Note:** Removed after taking damage once

---

## 6. SUPPORT EFFECTS

### SC_ENDURE
**Name:** Endure
**Effect:** MDEF +level; No flinch when attacked

**Usage:**
```c
sc_start SC_ENDURE,60000,10;  // 60 seconds, level 10
```

**val1:** Skill level (MDEF bonus)

---

### SC_ANGELUS
**Name:** Angelus
**Effect:** DEF +(5×level)%

**Usage:**
```c
sc_start SC_ANGELUS,300000,10;  // 5 minutes, level 10
```

---

### SC_KYRIE
**Name:** Kyrie Eleison
**Effect:** Block damage up to (MaxHP×(level×2+10)/100) or ((level/2)+5) times

**Usage:**
```c
sc_start SC_KYRIE,120000,10;  // 2 minutes, level 10
```

**Note:** Removes SC_ASSUMPTIO

---

### SC_MAGNIFICAT
**Name:** Magnificat
**Effect:** SP regeneration speed ×2

**Usage:**
```c
sc_start SC_MAGNIFICAT,30000,1;  // 30 seconds
```

---

### SC_SUFFRAGIUM
**Name:** Suffragium
**Effect:** Cast time -(15×level)%

**Usage:**
```c
sc_start SC_SUFFRAGIUM,30000,3;  // 30 seconds, level 3 (-45% cast time)
```

---

### SC_SLOWPOISON
**Name:** Slow Poison
**Effect:** Stop HP reduction from SC_POISON

**Usage:**
```c
sc_start SC_SLOWPOISON,120000,1;  // 2 minutes
```

**Note:** Doesn't cure poison, just stops HP loss

---

### SC_ENERGYCOAT
**Name:** Energy Coat
**Effect:** Damage reduction based on SP

**Usage:**
```c
sc_start SC_ENERGYCOAT,300000,5;  // 5 minutes
```

---

## 7. SPECIAL STATES

### SC_HIDING
**Name:** Hiding
**Effect:** Set OPTION_HIDE (invisible, can't be targeted)

**Usage:**
```c
sc_start SC_HIDING,300000,1;  // 5 minutes
```

**Note:** Broken by specific skills/items

---

### SC_CLOAKING
**Name:** Cloaking
**Effect:** Set OPTION_CLOAK (invisible while near walls)

**Usage:**
```c
sc_start SC_CLOAKING,300000,10;  // 5 minutes
```

---

### SC_TRICKDEAD
**Name:** Play Dead
**Effect:** HP/SP regen disabled; Removes dancing status

**Usage:**
```c
sc_start SC_TRICKDEAD,600000,1;  // 10 minutes
```

---

### SC_POISONREACT
**Name:** Poison React
**Effect:** Block poison attacks; Counter with Envenom 5

**Usage:**
```c
sc_start SC_POISONREACT,180000,10;  // 3 minutes
```

**val1:** Skill level
**val2:** Number of Envenom autocasts
**val3:** Autocast chance
**val4:** 0=Block mode, 1=Damage boost mode

---

## 8. ADVANCED USAGE

### sc_start Syntax

```c
sc_start <SC_CONSTANT>,<duration_ms>,<val1>{,<rate>,<flag>,<unit_id>};
```

**Parameters:**
- `SC_CONSTANT` - Status effect constant (e.g., SC_BLESSING)
- `duration_ms` - Duration in milliseconds
- `val1` - Primary value (usually skill level or intensity)
- `rate` - Success rate (10000 = 100%, optional, default 10000)
- `flag` - Special flags (optional)
  - `SCSTART_NOAVOID (1)` - Cannot be avoided
  - `SCSTART_NOTICKDEF (2)` - Ignore tick defense
  - `SCSTART_NOICON (4)` - Don't show status icon
  - `SCSTART_LOADED (8)` - Values pre-calculated
- `unit_id` - Target unit ID (optional, default = attached player)

---

### Examples

#### Apply Blessing with 100% Success
```c
sc_start SC_BLESSING,240000,10,10000;
```

#### Apply Poison with 50% Success Rate
```c
sc_start SC_POISON,60000,5,5000;
```

#### Apply Stone with No Icon
```c
sc_start SC_STONE,30000,1,10000,SCSTART_NOICON;
```

---

### sc_start2 / sc_start4

```c
// sc_start2: Specify val1 and val2
sc_start2 SC_POISONREACT,180000,10,5;

// sc_start4: Specify val1, val2, val3, val4
sc_start4 SC_POISON,60000,5,getcharid(3),0,gettick();
```

---

### Removing Status Effects

```c
// Remove specific status
sc_end SC_STONE;
sc_end SC_BLESSING;

// Remove all buffs
sc_end SC_ALL;
```

---

### Checking Status Effects

```c
// Check if status is active
if (getstatus(SC_STONE)) {
    mes "You are petrified!";
    sc_end SC_STONE;
}

// Check remaining time (in milliseconds)
.@time = getstatus(SC_BLESSING, 1);
mes "Blessing remaining: " + (.@time/1000) + " seconds";
```

---

## COMMON PATTERNS

### Buff NPC
```c
prontera,150,150,4	script	Buffer	4_F_KAFRA1,{
    mes "[Buffer]";
    mes "Choose your buffs:";
    next;
    switch(select("Full Buffs:Custom:Cancel")) {
    case 1:
        sc_start SC_BLESSING,240000,10;
        sc_start SC_INCREASEAGI,240000,10;
        sc_start SC_IMPOSITIO,240000,5;
        sc_start SC_GLORIA,240000,1;
        mes "Fully buffed!";
        close;
    case 2:
        // Custom menu here
        close;
    case 3:
        close;
    }
}
```

### Cure Ailments
```c
if (getstatus(SC_STONE) || getstatus(SC_FREEZE) || getstatus(SC_STUN)) {
    sc_end SC_STONE;
    sc_end SC_FREEZE;
    sc_end SC_STUN;
    mes "Ailments cured!";
}
```

### Temporary PvP Buff
```c
// 30 second combat buff
sc_start SC_TWOHANDQUICKEN,30000,10;
sc_start SC_OVERTHRUST,30000,5;
sc_start SC_WEAPONPERFECTION,30000,5;
announce "Combat buffs active for 30 seconds!",bc_self;
```

---

## DURATION REFERENCE

```c
// Common durations
1000      // 1 second
5000      // 5 seconds
10000     // 10 seconds
30000     // 30 seconds
60000     // 1 minute
120000    // 2 minutes
180000    // 3 minutes
240000    // 4 minutes
300000    // 5 minutes
600000    // 10 minutes
```

---

## TIPS & BEST PRACTICES

1. **Always use milliseconds** for duration (multiply seconds by 1000)
2. **Check existing status** before applying to avoid overwriting longer durations
3. **Use appropriate val1** values (usually skill level)
4. **Success rate** of 10000 = 100% guaranteed
5. **Combine buffs** for better player experience
6. **Clear debuffs** before important events
7. **Test durations** - some effects are very short/long
8. **Use constants** (SC_BLESSING) instead of numbers

---

## DEBUGGING

### Check All Active Status Effects
```c
for (.@i = 0; .@i < SC_MAX; .@i++) {
    if (getstatus(.@i)) {
        dispbottom "Active: " + .@i;
    }
}
```

### Display Status Duration
```c
.@time = getstatus(SC_BLESSING, 1);
if (.@time > 0) {
    dispbottom "Blessing: " + (.@time/1000) + "s remaining";
}
```

---

## SEE ALSO

- **Full Reference:** `/doc/status_change.txt` (2,920 lines, 300+ status effects)
- **KB_REF_005:** Script Commands Reference
- **KB_REF_007:** Visual Effects Reference
- **KB_EXAMPLE_001:** NPC Script Examples

---

*Last Updated: 2024-10-24*
*rAthena Documentation - Status Effects*
