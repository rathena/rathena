---
kb_id: KB_REF_008
kb_type: reference
kb_category: items
kb_subcategory: item_bonuses
kb_keywords: [item bonuses, bonus, bonus2, bonus3, bonus4, bonus5, equipment, item script, stats, damage, resist, autospell, constants, bStr, bAtk, bMaxHP]
kb_related: [KB_DB_001, KB_REF_005]
kb_difficulty: intermediate
kb_version: rAthena_2025
kb_last_updated: 2015-10-29
kb_use_case: [item_creation, equipment_scripts, custom_items, balance]
---

# rAthena Item Bonuses Reference

Complete reference for item bonus commands used in equipment scripts.

## Overview

Item bonuses are script commands used in item_db.yml Script fields to grant special properties to equipment. They're automatically applied when equipping and removed when unequipping.

**Usage Location:** `item_db.yml` → Script field

**Example:**
```yaml
- Id: 1234
  AegisName: Custom_Sword
  Name: Custom Sword
  Type: Weapon
  Script: |
    bonus bStr,10;
    bonus bAtk,50;
    bonus2 bAddRace,RC_Demon,20;
```

---

## TABLE OF CONTENTS

1. [Constants](#1-constants)
2. [Basic Stats](#2-basic-stats)
3. [HP/SP/AP](#3-hpspap)
4. [Attack & Defense](#4-attack--defense)
5. [Accuracy & Evasion](#5-accuracy--evasion)
6. [Damage Modifiers](#6-damage-modifiers)
7. [Resistance & Reduction](#7-resistance--reduction)
8. [AutoSpell](#8-autospell)
9. [Special Effects](#9-special-effects)
10. [Advanced Bonuses](#10-advanced-bonuses)

---

## 1. CONSTANTS

### Status Effects (eff)
```c
Eff_Bleeding, Eff_Blind, Eff_Burning, Eff_Confusion, Eff_Crystalize,
Eff_Curse, Eff_DPoison, Eff_Fear, Eff_Freeze, Eff_Poison, Eff_Silence,
Eff_Sleep, Eff_Stone, Eff_Stun, Eff_Freezing, Eff_Heat, Eff_Deepsleep,
Eff_WhiteImprison, Eff_Hallucination
```

---

### Elements (e)
```c
Ele_Dark, Ele_Earth, Ele_Fire, Ele_Ghost, Ele_Holy, Ele_Neutral,
Ele_Poison, Ele_Undead, Ele_Water, Ele_Wind, Ele_All
```

---

### Races (r)
```c
RC_Angel, RC_Brute, RC_DemiHuman, RC_Demon, RC_Dragon, RC_Fish,
RC_Formless, RC_Insect, RC_Plant, RC_Player_Human, RC_Player_Doram,
RC_Undead, RC_All
```

---

### Classes (c)
```c
Class_Normal, Class_Boss, Class_Guardian, Class_All
```

---

### Sizes (s)
```c
Size_Small, Size_Medium, Size_Large, Size_All
```

---

### Trigger Criteria (bf)

**Range:**
- `BF_SHORT` - Melee attacks
- `BF_LONG` - Ranged attacks

**Type:**
- `BF_WEAPON` - Weapon skills
- `BF_MAGIC` - Magic skills
- `BF_MISC` - Misc skills

**Attack Type:**
- `BF_NORMAL` - Normal attacks
- `BF_SKILL` - Skills

---

### Trigger Criteria (atf)

**Target:**
- `ATF_SELF` - Trigger on self
- `ATF_TARGET` - Trigger on target

**Range:**
- `ATF_SHORT` - Melee attacks
- `ATF_LONG` - Ranged attacks

**Type:**
- `ATF_WEAPON` - Physical attacks
- `ATF_SKILL` - Skills
- `ATF_MAGIC` - Magic skills
- `ATF_MISC` - Misc skills

---

## 2. BASIC STATS

### Base Stats

```c
bonus bStr,n;          // STR + n
bonus bAgi,n;          // AGI + n
bonus bVit,n;          // VIT + n
bonus bInt,n;          // INT + n
bonus bDex,n;          // DEX + n
bonus bLuk,n;          // LUK + n
bonus bAllStats,n;     // All stats + n
bonus bAgiVit,n;       // AGI + n, VIT + n
bonus bAgiDexStr,n;    // STR + n, AGI + n, DEX + n
```

**Examples:**
```yaml
# +10 STR sword
Script: |
  bonus bStr,10;

# +5 all stats armor
Script: |
  bonus bAllStats,5;

# AGI/VIT shoes
Script: |
  bonus bAgiVit,3;
```

---

### Trait Stats (4th Job)

```c
bonus bPow,n;          // POW + n
bonus bSta,n;          // STA + n
bonus bWis,n;          // WIS + n
bonus bSpl,n;          // SPL + n
bonus bCon,n;          // CON + n
bonus bCrt,n;          // CRT + n
bonus bAllTraitStats,n; // All trait stats + n
```

---

## 3. HP/SP/AP

```c
bonus bMaxHP,n;        // MaxHP + n
bonus bMaxHPrate,n;    // MaxHP + n%
bonus bMaxSP,n;        // MaxSP + n
bonus bMaxSPrate,n;    // MaxSP + n%
bonus bMaxAP,n;        // MaxAP + n (4th job)
bonus bMaxAPrate,n;    // MaxAP + n%
```

**Examples:**
```yaml
# +500 HP armor
Script: |
  bonus bMaxHP,500;

# +10% HP/SP accessory
Script: |
  bonus bMaxHPrate,10;
  bonus bMaxSPrate,10;
```

---

## 4. ATTACK & DEFENSE

### Attack

```c
bonus bBaseAtk,n;          // Base ATK + n
bonus bAtk,n;              // ATK + n
bonus bAtk2,n;             // ATK2 + n
bonus bAtkRate,n;          // ATK + n%
bonus bWeaponAtkRate,n;    // Weapon ATK + n%
bonus bMatk,n;             // MATK + n
bonus bMatk2,n;            // MATK + n (hidden)
bonus bMatkRate,n;         // MATK + n%
bonus bWeaponMatkRate,n;   // Weapon MATK + n%
```

**Examples:**
```yaml
# High damage sword
Script: |
  bonus bAtk,100;
  bonus bAtkRate,15;

# Magic staff
Script: |
  bonus bMatk,150;
  bonus bMatkRate,10;
```

---

### Defense

```c
bonus bDef,n;              // Equipment DEF + n
bonus bDefRate,n;          // Equipment DEF + n%
bonus bDef2,n;             // VIT-based DEF + n
bonus bDef2Rate,n;         // VIT-based DEF + n%
bonus bMdef,n;             // Equipment MDEF + n
bonus bMdefRate,n;         // Equipment MDEF + n%
bonus bMdef2,n;            // INT-based MDEF + n
bonus bMdef2Rate,n;        // INT-based MDEF + n%
```

**Examples:**
```yaml
# Tank shield
Script: |
  bonus bDef,50;
  bonus bMdef,20;

# % defense boost
Script: |
  bonus bDefRate,15;
```

---

## 5. ACCURACY & EVASION

```c
bonus bHit,n;              // HIT + n
bonus bHitRate,n;          // HIT + n%
bonus bCritical,n;         // CRIT + n
bonus bCriticalRate,n;     // CRIT + n%
bonus bFlee,n;             // FLEE + n
bonus bFleeRate,n;         // FLEE + n%
bonus bFlee2,n;            // Perfect Dodge + n
bonus bFlee2Rate,n;        // Perfect Dodge + n%
bonus bPerfectHitRate,n;   // Perfect Hit + n%
bonus bPerfectHitAddRate,n; // Perfect Hit + n%
```

**Examples:**
```yaml
# Accuracy gloves
Script: |
  bonus bHit,20;

# Critical dagger
Script: |
  bonus bCritical,15;
  bonus bCriticalRate,10;

# Evasion boots
Script: |
  bonus bFlee,15;
  bonus bFlee2,5;
```

---

## 6. DAMAGE MODIFIERS

### Race Damage

```c
bonus2 bAddRace,r,n;           // +n% damage vs race
bonus2 bMagicAddRace,r,n;      // +n% magic damage vs race
bonus2 bSubRace,r,n;           // -n% damage from race
bonus2 bSubRace2,mr,n;         // -n% damage from monster race
```

**Examples:**
```yaml
# Anti-Demon sword
Script: |
  bonus2 bAddRace,RC_Demon,20;
  bonus2 bAddRace,RC_Undead,20;

# Anti-Boss armor
Script: |
  bonus2 bSubRace,RC_All,10;
  if (readparam(bBaseLevel) >= 99) {
    bonus2 bSubRace,Class_Boss,15;
  }
```

---

### Element Damage

```c
bonus2 bAddEle,e,n;            // +n% damage vs element
bonus2 bMagicAddEle,e,n;       // +n% magic damage vs element
bonus2 bSubEle,e,n;            // -n% damage from element
bonus3 bAddEle,e,n,bf;         // +n% damage vs element with flags
```

**Examples:**
```yaml
# Fire damage sword
Script: |
  bonus2 bAddEle,Ele_Fire,25;

# Water resistance armor
Script: |
  bonus2 bSubEle,Ele_Water,20;
```

---

### Size Damage

```c
bonus2 bAddSize,s,n;           // +n% damage vs size
bonus2 bMagicAddSize,s,n;      // +n% magic damage vs size
bonus2 bSubSize,s,n;           // -n% damage from size
bonus bNoSizeFix;              // Ignore size penalty
```

**Examples:**
```yaml
# Large monster hunter
Script: |
  bonus2 bAddSize,Size_Large,20;

# Pike (no size penalty)
Script: |
  bonus bNoSizeFix;
```

---

### Class Damage

```c
bonus2 bAddClass,c,n;          // +n% damage vs class
bonus2 bMagicAddClass,c,n;     // +n% magic damage vs class
bonus2 bSubClass,c,n;          // -n% damage from class
```

**Examples:**
```yaml
# MVP killer weapon
Script: |
  bonus2 bAddClass,Class_Boss,30;
  bonus2 bAddClass,Class_Guardian,30;
```

---

## 7. RESISTANCE & REDUCTION

### Physical Reduction

```c
bonus bNearAtkDef,n;           // -n% damage from melee
bonus bLongAtkDef,n;           // -n% damage from ranged
bonus bWeaponAtkRate,n;        // +n% weapon attack
bonus bCritAtkRate,n;          // +n% critical damage
bonus bCriticalDef,n;          // -n critical rate from enemy
```

---

### Magic Reduction

```c
bonus bMagicDamageReturn,n;    // n% magic damage reflected
bonus bMagicAtkEle,e;          // Change magic element
```

---

### Status Resistance

```c
bonus2 bResEff,eff,n;          // +n% resistance to status
bonus2 bAddEffWhenHit,eff,n;   // n% chance to inflict status when hit
bonus3 bAddEff,eff,n,atf;      // n% chance to inflict status
```

**Examples:**
```yaml
# Stun immunity helm
Script: |
  bonus2 bResEff,Eff_Stun,10000;

# Poison resistance armor
Script: |
  bonus2 bResEff,Eff_Poison,5000;
  bonus2 bResEff,Eff_DPoison,5000;

# Curse on hit
Script: |
  bonus2 bAddEffWhenHit,Eff_Curse,1000;
```

---

## 8. AUTOSPELL

### Basic AutoSpell

```c
bonus4 bAutoSpell,sk,lv,rate,bf;  // Cast skill on attack
bonus4 bAutoSpellWhenHit,sk,lv,rate,bf;  // Cast skill when hit
bonus5 bAutoSpell,sk,lv,rate,bf,iid;  // With item cost
```

**Examples:**
```yaml
# Fire Bolt on attack (3% chance)
Script: |
  bonus4 bAutoSpell,MG_FIREBOLT,5,30,0;

# Heal when hit (5% chance)
Script: |
  bonus4 bAutoSpellWhenHit,AL_HEAL,10,50,0;

# Cold Bolt (long range only)
Script: |
  bonus4 bAutoSpell,MG_COLDBOLT,5,30,BF_LONG;
```

---

### Advanced AutoSpell

```c
bonus4 bAutoSpellWhenHit,sk,lv,rate,bf;
bonus5 bAutoSpellWhenHit,sk,lv,rate,bf,iid;
bonus5 bAutoSpell,sk,-lv,rate,bf,atf;  // Negative level = cast on self
```

**Examples:**
```yaml
# Blessing on self when hit
Script: |
  bonus5 bAutoSpellWhenHit,AL_BLESSING,-10,100,BF_WEAPON,ATF_SELF;

# Provoke on melee attack
Script: |
  bonus4 bAutoSpell,SM_PROVOKE,10,50,BF_SHORT;
```

---

## 9. SPECIAL EFFECTS

### HP/SP Drain

```c
bonus bHPDrainValue,n;         // Drain n HP per hit
bonus2 bHPDrainRate,n,m;       // Drain n% HP, max m per hit
bonus2 bSPDrainRate,n,m;       // Drain n% SP, max m per hit
bonus3 bHPDrainRate,n,m,r;     // Drain from specific race
```

**Examples:**
```yaml
# Vampire sword
Script: |
  bonus2 bHPDrainRate,5,100;  // 5% drain, max 100 HP

# SP drain accessory
Script: |
  bonus2 bSPDrainRate,2,50;   // 2% drain, max 50 SP
```

---

### SP Cost Reduction

```c
bonus bUseSPrate,n;            // SP cost +n% (use negative)
bonus2 bSkillUseSP,sk,n;       // Skill SP cost +n
bonus2 bSkillUseSPrate,sk,n;   // Skill SP cost +n%
```

**Examples:**
```yaml
# Reduce all SP costs by 20%
Script: |
  bonus bUseSPrate,-20;

# Reduce Heal cost by 50%
Script: |
  bonus2 bSkillUseSPrate,AL_HEAL,-50;
```

---

### Cast Time

```c
bonus bCastrate,n;             // Cast time +n% (use negative)
bonus2 bSkillCooldown,sk,n;    // Skill cooldown -n ms
bonus2 bSkillCast,sk,n;        // Skill cast time +n ms
bonus2 bVariableCastrate,sk,n; // Variable cast +n%
bonus2 bFixedCastrate,sk,n;    // Fixed cast +n%
```

**Examples:**
```yaml
# Instant cast accessory
Script: |
  bonus bCastrate,-100;

# Heal -1 second cooldown
Script: |
  bonus2 bSkillCooldown,AL_HEAL,-1000;
```

---

### ASPD & Movement

```c
bonus bAspd,n;                 // ASPD + n
bonus bAspdRate,n;             // ASPD + n%
bonus bSpeedRate,n;            // Movement speed +n%
bonus bSpeedAddRate,n;         // Movement speed +n%
```

**Examples:**
```yaml
# AGI boots
Script: |
  bonus bAspd,1;
  bonus bSpeedRate,25;
```

---

## 10. ADVANCED BONUSES

### Skill Damage

```c
bonus2 bSkillAtk,sk,n;         // Skill damage +n%
bonus2 bSkillHeal,sk,n;        // Skill heal +n%
bonus3 bAddEff,eff,n,atf;      // Add status effect chance
```

**Examples:**
```yaml
# Bash damage +50%
Script: |
  bonus2 bSkillAtk,SM_BASH,50;

# Heal effectiveness +30%
Script: |
  bonus2 bSkillHeal,AL_HEAL,30;
```

---

### Combo Bonuses

```c
bonus bComboAddBonus,iid;      // If equipped with item
```

**Example:**
```yaml
# Bonus when worn with Shield (2105)
Script: |
  bonus bStr,5;
  if (isequipped(2105)) {
    bonus bAtk,50;
    bonus bDef,20;
  }
```

---

### Conditional Bonuses

```c
if (condition) { bonus ...; }
```

**Examples:**
```yaml
# Level-based bonus
Script: |
  bonus bStr,5;
  if (readparam(bBaseLevel) >= 99) {
    bonus bStr,5;
    bonus bAtk,50;
  }

# Class-specific
Script: |
  bonus bAtk,50;
  if (Class == Job_Swordman) {
    bonus bAtk,30;
  }

# Time-based
Script: |
  bonus bAtk,50;
  if (gettime(DT_HOUR) >= 18 || gettime(DT_HOUR) < 6) {
    bonus bAtk,30;  // Night bonus
  }
```

---

## COMMON PATTERNS

### Tank Armor
```yaml
Script: |
  bonus bMaxHPrate,10;
  bonus bDef,30;
  bonus bMdef,20;
  bonus2 bSubRace,RC_All,5;
```

### DPS Weapon
```yaml
Script: |
  bonus bAtk,100;
  bonus bAtkRate,15;
  bonus bCritical,10;
  bonus bAspd,2;
```

### Magic Staff
```yaml
Script: |
  bonus bMatk,150;
  bonus bMatkRate,10;
  bonus bInt,5;
  bonus bCastrate,-10;
```

### MVP Killer
```yaml
Script: |
  bonus2 bAddClass,Class_Boss,30;
  bonus2 bAddClass,Class_Guardian,30;
  bonus2 bHPDrainRate,3,100;
```

### Support Accessory
```yaml
Script: |
  bonus bInt,5;
  bonus2 bSkillHeal,AL_HEAL,30;
  bonus2 bSkillUseSPrate,AL_HEAL,-25;
  bonus bUseSPrate,-10;
```

### PvP Armor
```yaml
Script: |
  bonus bMaxHPrate,15;
  bonus2 bSubRace,RC_Player_Human,10;
  bonus2 bSubRace,RC_Player_Doram,10;
  bonus2 bResEff,Eff_Stun,5000;
  bonus2 bResEff,Eff_Freeze,5000;
```

---

## BONUS COMMAND LEVELS

### bonus (1 parameter)
```c
bonus bStr,10;
```

### bonus2 (2 parameters)
```c
bonus2 bAddRace,RC_Demon,20;
```

### bonus3 (3 parameters)
```c
bonus3 bAddEff,Eff_Stun,500,ATF_SHORT;
```

### bonus4 (4 parameters)
```c
bonus4 bAutoSpell,MG_FIREBOLT,5,30,0;
```

### bonus5 (5 parameters)
```c
bonus5 bAutoSpell,MG_FIREBOLT,5,30,0,Red_Blood;
```

---

## DEBUGGING

### Test Equipment
```c
// Check if item bonuses apply
@item 1234  // Spawn item
// Equip and check stats with @stats

// Remove to verify bonus removal
@delitem 1234 1
```

### Display Bonus Values
```yaml
Script: |
  bonus bStr,10;
  dispbottom "STR +10 from equipment";
```

---

## TIPS & BEST PRACTICES

1. **Test thoroughly** - Bonuses stack in complex ways
2. **Balance carefully** - Too many bonuses = overpowered items
3. **Use constants** - RC_Demon instead of numbers
4. **Document bonuses** - Comment your script
5. **Consider combinations** - Multiple items with same bonus
6. **Check renewal mode** - Some bonuses work differently
7. **Validate IDs** - Skill IDs, item IDs must be correct
8. **Use conditionals** - Level/class-based bonuses for progression

---

## SEE ALSO

- **Full Reference:** `/doc/item_bonus.txt` (512 lines, 100+ bonus types)
- **KB_REF_005:** Script Commands Reference
- **KB_DB_001:** Item Database Structure
- **Item Database:** `/db/(pre-)re/item_db.yml`

---

*Last Updated: 2015-10-29*
*rAthena Documentation - Item Bonuses*
