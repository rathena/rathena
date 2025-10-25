---
kb_id: KB_REF_010
kb_type: reference
kb_category: scripting
kb_subcategory: npc_examples
kb_keywords: [npc examples, script examples, code patterns, quest scripts, shop scripts, buffer npc, warp npc, monster spawner, timer, array, function, common patterns]
kb_related: [KB_REF_005, KB_REF_002, KB_REF_003]
kb_difficulty: beginner
kb_version: rAthena_2025
kb_last_updated: 2025-10-23
kb_use_case: [npc_scripting, quest_creation, learning, templates]
---

# rAthena NPC Script Examples

Collection of working NPC script examples and common patterns.

## Overview

This document provides ready-to-use NPC script examples for common scenarios. Copy, modify, and learn from these patterns.

**File Location:** Save scripts in `/npc/custom/` directory
**Load In:** `/npc/scripts_custom.conf`

---

## TABLE OF CONTENTS

1. [Basic NPCs](#1-basic-npcs)
2. [Quest NPCs](#2-quest-npcs)
3. [Shop NPCs](#3-shop-npcs)
4. [Buffer NPCs](#4-buffer-npcs)
5. [Warper NPCs](#5-warper-npcs)
6. [Monster Spawners](#6-monster-spawners)
7. [Timers & Events](#7-timers--events)
8. [Advanced Patterns](#8-advanced-patterns)

---

## 1. BASIC NPCS

### Simple Dialogue NPC
```c
prontera,150,150,4	script	Greeter	4_F_KAFRA1,{
    mes "[Greeter]";
    mes "Hello, " + strcharinfo(0) + "!";
    mes "Welcome to our server!";
    close;
}
```

---

### NPC with Menu
```c
prontera,151,150,4	script	Helper	4_M_JOB_KNIGHT1,{
    mes "[Helper]";
    mes "How can I help you?";
    next;
    switch(select("Heal Me:Buff Me:Information:Cancel")) {
    case 1:
        percentheal 100,100;
        mes "Healed!";
        close;
    case 2:
        sc_start SC_BLESSING,240000,10;
        sc_start SC_INCREASEAGI,240000,10;
        mes "Buffed!";
        close;
    case 3:
        mes "This is a helper NPC.";
        mes "We can heal and buff you!";
        close;
    case 4:
        close;
    }
}
```

---

### NPC with Input
```c
prontera,152,150,4	script	Name Teller	4_F_TELEPORTER,{
    mes "[Name Teller]";
    mes "What's your name?";
    next;
    input .@name$;

    mes "[Name Teller]";
    if (.@name$ == "") {
        mes "You didn't enter a name!";
    } else {
        mes "Nice to meet you, " + .@name$ + "!";
    }
    close;
}
```

---

## 2. QUEST NPCS

### Simple Quest (Item Collection)
```c
prontera,153,150,4	script	Quest Giver	4_M_ALCHE_C,{
    mes "[Quest Giver]";

    // Check if quest already completed
    if (#PORING_QUEST) {
        mes "Thanks for helping before!";
        close;
    }

    // Check if player has items
    if (countitem(909) >= 10) {  // 10 Jellopy
        mes "You brought the Jellopy!";
        mes "Here's your reward.";
        delitem 909,10;
        getexp 1000,500;
        getitem 501,5;  // 5 Red Potions
        set #PORING_QUEST,1;
        close;
    }

    // Give quest
    mes "Can you bring me 10 Jellopy?";
    mes "I'll reward you well!";
    close;
}
```

---

### Hunt Quest (with quest_db)
```c
prontera,154,150,4	script	Hunt Quest	4_M_KNIGHT_GOLD,{
    .@quest_id = 1000;  // Quest ID from quest_db.yml

    mes "[Hunt Quest]";

    // Check quest status
    .@status = questprogress(.@quest_id, HUNTING);

    if (.@status == 0) {
        // Quest not started
        mes "Hunt 30 Porings for me!";
        next;
        if (select("Accept:Decline") == 2) close;
        setquest .@quest_id;
        mes "Good luck!";
        close;
    } else if (.@status == 1) {
        // Quest active but not complete
        mes "Keep hunting! You're not done yet.";
        close;
    } else if (.@status == 2) {
        // Quest complete
        mes "You finished! Here's your reward.";
        completequest .@quest_id;
        getexp 5000,2000;
        getitem 501,10;
        close;
    }
}
```

---

### Daily Quest (Time-based)
```c
prontera,155,150,4	script	Daily Quest	4_F_ALCHE,{
    mes "[Daily Quest]";

    // Check if already done today
    if (#DAILY_DONE >= gettimetick(1)) {
        .@hours = ((#DAILY_DONE - gettimetick(1)) / 3600);
        mes "Come back in " + .@hours + " hours!";
        close;
    }

    // Check items
    if (countitem(501) >= 20) {  // 20 Red Potions
        mes "Thanks! Come back tomorrow!";
        delitem 501,20;
        getexp 10000,5000;
        set #DAILY_DONE, gettimetick(1) + 86400;  // 24 hours
        close;
    }

    mes "Bring me 20 Red Potions!";
    mes "Resets daily.";
    close;
}
```

---

## 3. SHOP NPCS

### Basic Shop
```c
prontera,156,150,4	shop	Potion Shop	4_F_01,501:50,502:200,503:500
```

---

### Custom Shop (Script-based)
```c
prontera,157,150,4	script	Custom Shop	4_M_ORIENT02,{
    mes "[Custom Shop]";
    mes "What do you need?";
    next;

    switch(select("Red Potion - 50z:Blue Potion - 200z:White Potion - 500z:Cancel")) {
    case 1:
        if (Zeny < 50) {
            mes "Not enough Zeny!";
            close;
        }
        Zeny -= 50;
        getitem 501,1;
        mes "Here's your Red Potion!";
        close;
    case 2:
        if (Zeny < 200) {
            mes "Not enough Zeny!";
            close;
        }
        Zeny -= 200;
        getitem 505,1;
        mes "Here's your Blue Potion!";
        close;
    case 3:
        if (Zeny < 500) {
            mes "Not enough Zeny!";
            close;
        }
        Zeny -= 500;
        getitem 504,1;
        mes "Here's your White Potion!";
        close;
    case 4:
        close;
    }
}
```

---

### Level-based Shop
```c
prontera,158,150,4	script	VIP Shop	4_M_SAGE_C,{
    mes "[VIP Shop]";

    if (BaseLevel < 50) {
        mes "Sorry, level 50+ only!";
        close;
    }

    mes "Welcome, high-level adventurer!";
    next;
    callshop "vip_items",1;
    npcshopattach "vip_items";
    end;

OnBuyItem:
    // Custom buy logic here
    end;

OnSellItem:
    // Custom sell logic here
    end;
}

-	shop	vip_items	-1,501:25,502:100,503:250
```

---

## 4. BUFFER NPCS

### Basic Buffer
```c
prontera,159,150,4	script	Buffer	4_F_KAFRA1,{
    mes "[Buffer]";
    mes "Free buffs!";
    next;

    sc_start SC_BLESSING,240000,10;
    sc_start SC_INCREASEAGI,240000,10;
    sc_start SC_IMPOSITIO,240000,5;
    sc_start SC_GLORIA,240000,1;

    specialeffect2 EF_BLESSING;

    mes "Buffed!";
    close;
}
```

---

### Menu-based Buffer
```c
prontera,160,150,4	script	Advanced Buffer	4_F_KAFRA2,{
    mes "[Advanced Buffer]";
    mes "Choose your buffs:";
    next;

    setarray .@buffs$[0],
        "Blessing (+STR/INT/DEX)",
        "Increase AGI (+AGI/Speed)",
        "Impositio (+ATK)",
        "Gloria (+LUK)",
        "Kyrie Eleison (Barrier)",
        "Magnificat (SP Regen)",
        "All Buffs",
        "Cancel";

    .@choice = select(implode(.@buffs$,":")) - 1;

    switch (.@choice) {
    case 0:
        sc_start SC_BLESSING,240000,10;
        break;
    case 1:
        sc_start SC_INCREASEAGI,240000,10;
        break;
    case 2:
        sc_start SC_IMPOSITIO,240000,5;
        break;
    case 3:
        sc_start SC_GLORIA,240000,1;
        break;
    case 4:
        sc_start SC_KYRIE,120000,10;
        break;
    case 5:
        sc_start SC_MAGNIFICAT,30000,1;
        break;
    case 6:
        sc_start SC_BLESSING,240000,10;
        sc_start SC_INCREASEAGI,240000,10;
        sc_start SC_IMPOSITIO,240000,5;
        sc_start SC_GLORIA,240000,1;
        sc_start SC_KYRIE,120000,10;
        sc_start SC_MAGNIFICAT,30000,1;
        break;
    case 7:
        close;
    }

    specialeffect2 EF_BLESSING;
    mes "Buff applied!";
    close;
}
```

---

## 5. WARPER NPCS

### Basic Warper
```c
prontera,161,150,4	script	Warper	4_M_TELEPORTER,{
    mes "[Warper]";
    mes "Where do you want to go?";
    next;

    switch(select("Prontera:Geffen:Payon:Morocc:Cancel")) {
    case 1:
        warp "prontera",156,191;
        end;
    case 2:
        warp "geffen",120,100;
        end;
    case 3:
        warp "payon",152,75;
        end;
    case 4:
        warp "morocc",156,93;
        end;
    case 5:
        close;
    }
}
```

---

### Categorized Warper
```c
prontera,162,150,4	script	Advanced Warper	4_M_TELEPORTER,{
    mes "[Advanced Warper]";
    mes "Select category:";
    next;

    switch(select("Cities:Dungeons:Fields:Cancel")) {
    case 1:  // Cities
        mes "[Warper]";
        mes "Which city?";
        next;
        switch(select("Prontera:Geffen:Payon:Morocc:Aldebaran:Back")) {
        case 1: warp "prontera",156,191; end;
        case 2: warp "geffen",120,100; end;
        case 3: warp "payon",152,75; end;
        case 4: warp "morocc",156,93; end;
        case 5: warp "aldebaran",140,131; end;
        case 6: close;
        }

    case 2:  // Dungeons
        mes "[Warper]";
        mes "Which dungeon?";
        next;
        switch(select("Prontera Culvert:Geffen Dungeon:Payon Cave:Back")) {
        case 1: warp "prt_sewb1",131,247; end;
        case 2: warp "gef_dun00",104,99; end;
        case 3: warp "pay_dun00",21,183; end;
        case 4: close;
        }

    case 3:  // Fields
        mes "[Warper]";
        mes "Which field?";
        next;
        switch(select("Prontera Field:Geffen Field:Payon Forest:Back")) {
        case 1: warp "prt_fild08",170,371; end;
        case 2: warp "gef_fild00",46,199; end;
        case 3: warp "pay_fild01",151,171; end;
        case 4: close;
        }

    case 4:
        close;
    }
}
```

---

## 6. MONSTER SPAWNERS

### Simple Spawner
```c
prontera,163,150,4	script	Spawn Poring	4_M_KID1,{
    mes "[Spawner]";
    mes "Spawn 5 Porings?";
    next;
    if (select("Yes:No") == 2) close;

    monster "prontera",0,0,"Poring",1002,5,"Spawner::OnPoringKilled";
    mes "Spawned!";
    close;

OnPoringKilled:
    announce "A Poring was killed!",bc_map;
    end;
}
```

---

### Timed Spawner
```c
-	script	AutoSpawner	-1,{
OnInit:
    // Spawn every 10 seconds
    initnpctimer;
    end;

OnTimer10000:
    monster "prontera",0,0,"Poring",1002,10;
    initnpctimer;  // Restart timer
    end;
}
```

---

### Wave Spawner
```c
prontera,164,150,4	script	Wave Event	4_M_MANAGER,{
    mes "[Wave Event]";
    mes "Start monster waves?";
    next;
    if (select("Yes:No") == 2) close;

    donpcevent "WaveController::OnWave1";
    mes "Wave 1 starting!";
    close;
}

-	script	WaveController	-1,{
OnWave1:
    announce "Wave 1: 10 Porings!",bc_map;
    monster "prontera",0,0,"Poring",1002,10,"WaveController::OnWave1Clear";
    end;

OnWave1Clear:
    if (mobcount("prontera","WaveController::OnWave1Clear") == 0) {
        sleep 3000;
        announce "Wave 2: 15 Drops!",bc_map;
        monster "prontera",0,0,"Drops",1113,15,"WaveController::OnWave2Clear";
    }
    end;

OnWave2Clear:
    if (mobcount("prontera","WaveController::OnWave2Clear") == 0) {
        announce "All waves complete!",bc_map;
        announce "Rewards distributed!",bc_map;
    }
    end;
}
```

---

## 7. TIMERS & EVENTS

### Announcement Timer
```c
-	script	HourlyAnnounce	-1,{
OnInit:
    initnpctimer;
    end;

OnTimer3600000:  // Every hour (3600000ms)
    announce "Server is running smoothly!",bc_all;
    initnpctimer;
    end;
}
```

---

### Event Scheduler
```c
-	script	EventScheduler	-1,{
OnClock2000:  // 8:00 PM
    announce "PvP Event starting in 5 minutes!",bc_all;
    sleep 300000;  // 5 minutes
    announce "PvP Event started!",bc_all;
    setmapflag "pvp_n_1-1",mf_pvp;
    end;

OnClock2100:  // 9:00 PM
    announce "PvP Event ended!",bc_all;
    removemapflag "pvp_n_1-1",mf_pvp;
    end;
}
```

---

### Login Reward
```c
-	script	LoginReward	-1,{
OnPCLoginEvent:
    // Check if already claimed today
    if (#LAST_LOGIN >= gettimetick(1)) end;

    announce "Welcome back! Daily reward claimed!",bc_self;
    getitem 501,10;  // 10 Red Potions
    set #LAST_LOGIN, gettimetick(1) + 86400;  // Next day
    end;
}
```

---

## 8. ADVANCED PATTERNS

### Function Usage
```c
function	script	GiveReward	{
    .@reward_exp = getarg(0);
    .@reward_zeny = getarg(1);

    getexp .@reward_exp,(.@reward_exp/2);
    Zeny += .@reward_zeny;

    mes "Received:";
    mes "- " + .@reward_exp + " Base EXP";
    mes "- " + (.@reward_exp/2) + " Job EXP";
    mes "- " + .@reward_zeny + " Zeny";
    return;
}

prontera,165,150,4	script	Reward NPC	4_M_ALCHE_C,{
    mes "[Reward NPC]";
    mes "Here's your reward!";
    next;

    callfunc("GiveReward",1000,500);
    close;
}
```

---

### Array Loop Example
```c
prontera,166,150,4	script	Item Giver	4_F_ALCHE,{
    mes "[Item Giver]";
    mes "Starter pack!";
    next;

    // Array of items
    setarray .@items[0],501,502,503,504,505;
    setarray .@amounts[0],10,10,5,5,3;

    for (.@i = 0; .@i < getarraysize(.@items); .@i++) {
        getitem .@items[.@i], .@amounts[.@i];
        mes "- " + getitemname(.@items[.@i]) + " x" + .@amounts[.@i];
    }
    close;
}
```

---

### Variable NPC (Changes based on server variable)
```c
prontera,167,150,4	script	Event Status	4_BOARD3,{
    mes "[Event Status]";

    if ($EVENT_ACTIVE) {
        mes "^00FF00Event is ACTIVE!^000000";
        mes "Bonus EXP: ^FF0000+" + $EVENT_BONUS + "%^000000";
    } else {
        mes "^FF0000No event active.^000000";
    }
    close;
}

prontera,168,150,4	script	Event Controller	4_M_MANAGER,{
    mes "[Event Controller]";
    mes "Control event status?";
    next;

    if (getgmlevel() < 90) {
        mes "GM only!";
        close;
    }

    switch(select("Start Event:Stop Event:Set Bonus")) {
    case 1:
        set $EVENT_ACTIVE,1;
        announce "Server Event Started! +" + $EVENT_BONUS + "% EXP!",bc_all;
        close;
    case 2:
        set $EVENT_ACTIVE,0;
        announce "Server Event Ended!",bc_all;
        close;
    case 3:
        mes "Enter bonus % (1-500):";
        input .@bonus,1,500;
        set $EVENT_BONUS,.@bonus;
        mes "Bonus set to " + .@bonus + "%!";
        close;
    }
}
```

---

### Instance Dungeon Entrance
```c
prontera,169,150,4	script	Instance Entrance	4_M_ALCHE_D,{
    .@instance$ = "MyInstance";
    .@instance_id = instance_id(IM_PARTY);

    mes "[Instance Entrance]";

    // Check if party leader
    if (getpartyleader(getcharid(1),2) != getcharid(0)) {
        mes "Only party leader can create instance.";
        close;
    }

    // Check if instance exists
    if (.@instance_id < 0) {
        mes "Create instance?";
        next;
        if (select("Yes:No") == 2) close;

        .@instance_id = instance_create(.@instance$,getcharid(1),IM_PARTY);
        if (.@instance_id < 0) {
            mes "Failed to create instance!";
            close;
        }

        if (instance_attachmap("1@tower",.@instance_id) == "") {
            mes "Failed to attach map!";
            instance_destroy(.@instance_id);
            close;
        }

        instance_init(.@instance_id);
        mes "Instance created!";
        close;
    }

    // Warp to instance
    mes "Enter instance?";
    next;
    if (select("Yes:No") == 2) close;

    warp instance_mapname("1@tower",.@instance_id),100,100;
    end;
}
```

---

## TIPS & BEST PRACTICES

1. **Always test scripts** before deploying to live server
2. **Use comments** to document complex logic
3. **Follow naming conventions** - Meaningful NPC names
4. **Check requirements** - Level, quest, items before giving rewards
5. **Validate input** - Use min/max in input commands
6. **Use arrays** for multiple similar items
7. **Add visual effects** for better player experience
8. **Handle edge cases** - What if player disconnects?
9. **Use functions** for repeated code
10. **Test with multiple players** for race conditions

---

## DEBUGGING NPCS

```c
// Add debug messages
dispbottom "Debug: Variable = " + .@var;
debugmes "Server debug: " + .@var;

// Check NPC status
@npctalk Test message  // Make NPC say something

// Reload scripts
@reloadscript  // Reload all scripts
```

---

## SEE ALSO

- **KB_REF_005:** Script Commands Reference
- **KB_REF_002:** Quest System
- **KB_REF_006:** Status Effects Reference
- **Sample Scripts:** `/doc/sample/` directory

---

*Last Updated: 2025-10-23*
*rAthena Documentation - NPC Script Examples*
