---
kb_id: KB_REF_007
kb_type: reference
kb_category: visual
kb_subcategory: effects
kb_keywords: [visual effects, effect, EF_, specialeffect, specialeffect2, client effects, animations, particles, sound effects, @effect]
kb_related: [KB_REF_005, KB_REF_006]
kb_difficulty: basic
kb_version: rAthena_2025
kb_last_updated: 2025-10-23
kb_use_case: [npc_scripting, item_effects, skill_effects, visual_enhancement]
---

# rAthena Visual Effects Reference

Complete reference for the most commonly used client-side visual effects in rAthena.

## Overview

Visual effects (also called "client effects") are animations, particles, and sounds that appear on the client screen. They enhance the player experience and provide visual feedback for actions, buffs, skills, and events.

**Script Usage:**
```c
// Show effect on NPC
specialeffect EF_MVP;

// Show effect on player
specialeffect2 EF_HEAL;

// Show effect on specific player by ID
specialeffect(EF_BLESSING, AREA, <account_id>);
```

**Testing:**
```c
// In-game command to test effects
@effect <number>
```

**Total Effects:** 968+ effects available (IDs 0-967+)

---

## TABLE OF CONTENTS

1. [Combat & Hit Effects](#1-combat--hit-effects)
2. [Magic & Elemental Effects](#2-magic--elemental-effects)
3. [Buff & Support Effects](#3-buff--support-effects)
4. [Status & Ailment Effects](#4-status--ailment-effects)
5. [Movement & Warp Effects](#5-movement--warp-effects)
6. [Special & Event Effects](#6-special--event-effects)
7. [Auras & Glows](#7-auras--glows)
8. [Usage Examples](#8-usage-examples)

---

## 1. COMBAT & HIT EFFECTS

### Basic Hits

| ID | Constant | Description |
|----|----------|-------------|
| 0 | EF_HIT1 | Regular Hit |
| 1 | EF_HIT2 | Bash |
| 2-5 | EF_HIT3-6 | Melee Skill Hits |
| 81 | EF_PIERCE | Pierce Hit |
| 17 | EF_MAGNUMBREAK | Magnum Break |
| 70 | EF_BRANDISHSPEAR | Brandish Spear |
| 73 | EF_BOWLINGBASH | Bowling Bash (Blue/White Aura) |

**Usage:**
```c
specialeffect2 EF_HIT1;  // Show hit on player
specialeffect EF_MAGNUMBREAK;  // Show on NPC
```

---

### Projectiles & Attacks

| ID | Constant | Description |
|----|----------|-------------|
| 64 | EF_ARROWSHOT | Arrow Shot (Purple/Yellow Light) |
| 80 | EF_SPEARBMR | Spear Boomerang |
| 49 | EF_FIREHIT | Firebolt/Wall Hit |
| 50 | EF_FIRESPLASHHIT | Spinning Fire |
| 51 | EF_COLDHIT | Ice Elemental Hit |
| 52 | EF_WINDHIT | Wind Elemental Hit |
| 53 | EF_POISONHIT | Poison Hit (Purple Smoke) |

---

## 2. MAGIC & ELEMENTAL EFFECTS

### Fire Spells

| ID | Constant | Description |
|----|----------|-------------|
| 24 | EF_FIREBALL | Fire Ball |
| 25 | EF_FIREWALL | Fire Wall |
| 214 | EF_LORD | Lord of Vermillion |
| 168 | EF_METEORSTORM | Meteor Storm |

**Usage:**
```c
specialeffect EF_FIREBALL;
sleep 500;
specialeffect EF_FIREHIT;
```

---

### Ice/Water Spells

| ID | Constant | Description |
|----|----------|-------------|
| 27 | EF_FROSTDIVER | Frost Diver (Traveling) |
| 28 | EF_FROSTDIVER2 | Frost Diver (Impact) |
| 74 | EF_ICEWALL | Ice Wall |
| 89 | EF_STORMGUST | Storm Gust |
| 107 | EF_WATERBALL | Water Ball |

---

### Lightning/Wind Spells

| ID | Constant | Description |
|----|----------|-------------|
| 29 | EF_LIGHTBOLT | Lightning Bolt |
| 30 | EF_THUNDERSTORM | Thunder Storm |
| 214 | EF_LORD | Lord of Vermillion |

---

### Earth Spells

| ID | Constant | Description |
|----|----------|-------------|
| 79 | EF_EARTHSPIKE | Earth Spike |
| 131 | EF_EARTHHIT | Earth Hit |
| 119 | EF_QUAGMIRE | Quagmire |

---

### Holy/Shadow Magic

| ID | Constant | Description |
|----|----------|-------------|
| 15 | EF_SOULSTRIKE | Soul Strike |
| 82 | EF_TURNUNDEAD | Turn Undead |
| 83 | EF_SANCTUARY | Sanctuary |
| 108 | EF_MAGNUS | Magnus Exorcismus |
| 33 | EF_RUWACH | Ruwach |

---

### Cast Auras

| ID | Constant | Description |
|----|----------|-------------|
| 12 | EF_BEGINSPELL | Yellow Cast Aura |
| 54 | EF_BEGINSPELL2 | Water Element Cast |
| 55 | EF_BEGINSPELL3 | Fire Element Cast |
| 56 | EF_BEGINSPELL4 | Earth Element Cast |
| 57 | EF_BEGINSPELL5 | Wind Element Cast |
| 58 | EF_BEGINSPELL6 | Holy Element Cast |
| 59 | EF_BEGINSPELL7 | Poison Element Cast |

**Usage:**
```c
specialeffect2 EF_BEGINSPELL3;  // Fire cast aura
sleep 2000;
specialeffect2 EF_FIREBALL;
```

---

## 3. BUFF & SUPPORT EFFECTS

### Stat Buffs

| ID | Constant | Description |
|----|----------|-------------|
| 42 | EF_BLESSING | Blessing |
| 37 | EF_INCAGILITY | AGI Up |
| 38 | EF_DECAGILITY | AGI Down |
| 43 | EF_INCAGIDEX | Dex + Agi Up |
| 75 | EF_GLORIA | Gloria |
| 67 | EF_PROVOKE | Provoke |

---

### Support Skills

| ID | Constant | Description |
|----|----------|-------------|
| 41 | EF_ANGELUS | Angelus |
| 76 | EF_MAGNIFICAT | Magnificat |
| 84 | EF_IMPOSITIO | Impositio Manus |
| 85 | EF_LEXAETERNA | Lex Aeterna |
| 86 | EF_ASPERSIO | Aspersio |
| 87 | EF_LEXDIVINA | Lex Divina |
| 88 | EF_SUFFRAGIUM | Suffragium |
| 11 | EF_ENDURE | Endure |

---

### Healing

| ID | Constant | Description |
|----|----------|-------------|
| 7 | EF_EXIT | Item Heal Effect |
| 9 | EF_ENHANCE | Different Heal Type |
| 14 | EF_HEALSP | Blue Restoring (SP) |
| 66 | EF_CURE | Cure |
| 77 | EF_RESURRECTION | Resurrection |
| 78 | EF_RECOVERY | Status Recovery |

**Usage:**
```c
// Healing effect
specialeffect2 EF_EXIT;
percentheal 100,100;
```

---

## 4. STATUS & AILMENT EFFECTS

### Status Ailments

| ID | Constant | Description |
|----|----------|-------------|
| 23 | EF_STONECURSE | Stone Curse |
| 20 | EF_PATTACK | Envenom/Poison |
| 21 | EF_DETOXICATION | Detoxify |
| 18 | EF_STEAL | Steal |
| 40 | EF_SIGNUM | Signum Crucis |
| 69 | EF_SKIDTRAP | Skid Trap |

---

### Visual Indicators

| ID | Constant | Description |
|----|----------|-------------|
| 60 | EF_LOCKON | Cast Target Circle |
| 22 | EF_SIGHT | Sight |
| 62 | EF_SIGHTRASHER | Sight Rasher |
| 66 | EF_CURE | Cure |

---

## 5. MOVEMENT & WARP EFFECTS

### Warp & Teleport

| ID | Constant | Description |
|----|----------|-------------|
| 6 | EF_ENTRY | Being Warped |
| 8 | EF_WARP | Yellow Ripple Effect |
| 34 | EF_TELEPORTATION | Teleport Animation |
| 35 | EF_READYPORTAL | Warp Portal (Ready) |
| 36 | EF_PORTAL | Warp Portal |
| 61 | EF_WARPZONE | NPC Warp |

**Usage:**
```c
specialeffect2 EF_WARP;
sleep 500;
warp "prontera",156,191;
```

---

### Movement Effects

| ID | Constant | Description |
|----|----------|-------------|
| 16 | EF_BASH | Hide |
| 69 | EF_SKIDTRAP | Skid Trap |

---

## 6. SPECIAL & EVENT EFFECTS

### MVP & Special

| ID | Constant | Description |
|----|----------|-------------|
| 68 | EF_MVP | MVP Banner |
| 10 | EF_COIN | Mammonite (Coins) |
| 63 | EF_BARRIER | Moonlight Sphere |
| 65 | EF_INVENOM | Power Absorb |

---

### Environmental

| ID | Constant | Description |
|----|----------|-------------|
| 44 | EF_SMOKE | Little Fog Smoke |
| 45 | EF_FIREFLY | Faint Little Balls |
| 46 | EF_SANDWIND | Sand Wind |
| 47 | EF_TORCH | Torch |
| 48 | EF_SPRAYPOND | Small Glass Piece |

---

### Geometric Effects

| ID | Constant | Description |
|----|----------|-------------|
| 13 | EF_GLASSWALL | Blue Box |
| 71 | EF_CONE | Spiral White Balls (Small) |
| 72 | EF_SPHERE | Spiral White Balls (Large) |

---

## 7. AURAS & GLOWS

### Light Effects

| ID | Constant | Description |
|----|----------|-------------|
| 132 | EF_LIGHTBLADE | Light Blade Aura |
| 244 | EF_GLASSWALL3 | Glass Wall (Clear) |
| 336 | EF_LIGHTSPHERE | Light Sphere |

---

### Colored Auras

| ID | Constant | Description |
|----|----------|-------------|
| 321 | EF_AURA_RED | Red Aura |
| 322 | EF_AURA_BLUE | Blue Aura |
| 323 | EF_AURA_GREEN | Green Aura |
| 324 | EF_AURA_YELLOW | Yellow Aura |

---

## 8. USAGE EXAMPLES

### Basic Effect Commands

```c
// Show effect on NPC location
specialeffect EF_MVP;

// Show effect on player
specialeffect2 EF_BLESSING;

// Show effect on specific player by account ID
specialeffect(EF_HEAL, AREA, getcharid(3));

// Show effect on specific coordinate
specialeffect(EF_FIREBALL, AREA, "prontera", 150, 150);
```

---

### Multiple Effects Sequence

```c
// Cast spell with visual feedback
specialeffect2 EF_BEGINSPELL3;  // Fire cast aura
sleep 2000;
specialeffect2 EF_FIREBALL;     // Fire ball
sleep 500;
specialeffect2 EF_FIREHIT;      // Impact
```

---

### Buff Application

```c
// Apply blessing with effect
specialeffect2 EF_BLESSING;
sc_start SC_BLESSING,240000,10;
mes "You have been blessed!";
```

---

### Healing with Effect

```c
// Heal player with visual feedback
specialeffect2 EF_EXIT;
percentheal 100,100;
mes "Fully healed!";
```

---

### Warp with Effect

```c
// Warp with visual effect
mes "Warping you now!";
close2;
specialeffect2 EF_WARP;
sleep 1000;
warp "prontera",156,191;
end;
```

---

### MVP Kill Effect

```c
// Announce MVP kill with effect
OnMVPDead:
    announce strcharinfo(0) + " has killed the MVP!",bc_all;
    specialeffect2 EF_MVP, AREA, getcharid(3);
    end;
```

---

### Area Effect

```c
// Show effect to all players in area
- script AreaEffect -1,{
OnInit:
    while (1) {
        specialeffect(EF_SANCTUARY, AREA, "prontera", 156, 191);
        sleep 5000;
    }
}
```

---

### Quest Complete Effect

```c
// Quest completion with celebration
mes "[Quest NPC]";
mes "Quest complete!";
close2;
specialeffect2 EF_MVP;
specialeffect2 EF_RESURRECTION;
getexp 10000,5000;
end;
```

---

### Buff NPC with Effects

```c
prontera,150,150,4	script	Buffer	4_F_KAFRA1,{
    mes "[Buffer]";
    mes "Would you like buffs?";
    next;
    if (select("Yes:No") == 2) close;

    // Visual feedback for each buff
    mes "Blessing...";
    specialeffect2 EF_BLESSING;
    sc_start SC_BLESSING,240000,10;
    sleep 500;

    mes "Increase AGI...";
    specialeffect2 EF_INCAGILITY;
    sc_start SC_INCREASEAGI,240000,10;
    sleep 500;

    mes "All done!";
    specialeffect2 EF_MAGNIFICAT;
    close;
}
```

---

### Item Effect Script

```c
// In item_db.yml Script field
Script: |
  specialeffect2 EF_EXIT;
  percentheal 100,100;
```

---

### Continuous Effect Loop

```c
// Create ambient effect
- script AmbientEffects -1,{
OnInit:
    initnpctimer;
    end;

OnTimer5000:
    specialeffect(EF_FIREFLY, AREA, "prontera", 156, 191);
    initnpctimer;
    end;
}
```

---

## EFFECT FLAGS

When using `specialeffect(effect, flag, ...)`:

**Flags:**
- `AREA` (0) - Show to all players in area
- `SELF` (1) - Show to invoking player only

```c
// Show to everyone
specialeffect(EF_MVP, AREA);

// Show to self only
specialeffect(EF_BLESSING, SELF);
```

---

## TESTING EFFECTS

### In-Game Testing

```c
// As GM, use @effect command
@effect 68  // Test EF_MVP

// Loop through effects to find what you want
@effect 0
@effect 1
@effect 2
// ... etc
```

---

### Test Script

```c
prontera,155,185,4	script	Effect Tester	4_M_ALCHE_C,{
    mes "[Effect Tester]";
    mes "Enter effect number (0-967):";
    input .@effect,0,967;
    specialeffect2 .@effect;
    dispbottom "Effect " + .@effect + " displayed.";
    close;
}
```

---

## PERFORMANCE TIPS

1. **Don't spam effects** - Can cause client lag
2. **Use sleep between effects** - Prevents visual overload
3. **Test effects** before deploying - Some look better than others
4. **Consider client version** - Older clients have fewer effects
5. **Use appropriate effects** - Match effect to action (heal for heal, fire for fire, etc.)
6. **Optimize loops** - Don't create continuous effect loops on busy maps

---

## COMMON EFFECT COMBINATIONS

### Level Up Celebration
```c
specialeffect2 EF_MVP;
specialeffect2 EF_RESURRECTION;
specialeffect2 EF_GLORIA;
```

### Magic Attack Sequence
```c
specialeffect2 EF_BEGINSPELL3;  // Cast
sleep 1500;
specialeffect EF_FIREBALL;       // Projectile
sleep 500;
specialeffect EF_FIREHIT;        // Impact
```

### Buff Combo
```c
specialeffect2 EF_BLESSING;
specialeffect2 EF_INCAGILITY;
specialeffect2 EF_MAGNIFICAT;
```

### Warp Sequence
```c
specialeffect2 EF_BEGINSPELL;
sleep 2000;
specialeffect2 EF_WARP;
sleep 500;
// warp here
```

---

## EFFECT CATEGORIES SUMMARY

| Category | Effect Count | ID Range |
|----------|--------------|----------|
| Combat | 50+ | 0-100 |
| Magic | 80+ | 15-250 |
| Buffs | 40+ | 37-88 |
| Status | 30+ | 18-120 |
| Movement | 10+ | 6-69 |
| Special | 100+ | 68-400 |
| Modern (Extended) | 568+ | 400-967 |

---

## SEE ALSO

- **Full List:** `/doc/effect_list.md` (968+ effects)
- **KB_REF_005:** Script Commands Reference
- **KB_REF_006:** Status Effects Reference
- **KB_EXAMPLE_001:** NPC Script Examples

---

## DEBUGGING

### Display Effect Number
```c
dispbottom "Showing effect: EF_MVP (" + EF_MVP + ")";
specialeffect2 EF_MVP;
```

### Log All Effects
```c
for (.@i = 0; .@i < 100; .@i++) {
    dispbottom "Effect " + .@i;
    specialeffect2 .@i;
    sleep 1000;
}
```

---

*Last Updated: 2025-10-23*
*rAthena Documentation - Visual Effects*
