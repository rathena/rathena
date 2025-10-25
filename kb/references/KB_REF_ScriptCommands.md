---
kb_id: KB_REF_005
kb_type: reference
kb_category: scripting
kb_subcategory: script_commands
kb_keywords: [script, npc, commands, scripting language, mes, next, close, menu, select, if, while, for, getitem, delitem, warp, heal, sc_start, monster, announce, variables, arrays, functions]
kb_related: [KB_EXAMPLE_001, KB_REF_008]
kb_difficulty: intermediate
kb_version: rAthena_2025
kb_last_updated: 2025-10-23
kb_use_case: [npc_scripting, quest_creation, custom_content, server_features]
---

# rAthena Essential Script Commands Reference

Comprehensive reference for the most commonly used rAthena scripting commands.

## Overview

This document covers the **essential 80+ commands** you'll use in 90% of NPC scripting. For the complete command list (400+ commands), see `/doc/script_commands.txt`.

**Script Basics:**
- Scripts are case-insensitive
- Commands end with semicolon (`;`)
- Use `//` for single-line comments
- Use `/* */` for multi-line comments
- All numbers are integers (no decimals)

---

## TABLE OF CONTENTS

1. [Dialogue & Display](#1-dialogue--display)
2. [Flow Control](#2-flow-control)
3. [Variables & Data](#3-variables--data)
4. [Player Information](#4-player-information)
5. [Items & Inventory](#5-items--inventory)
6. [Player Stats & Status](#6-player-stats--status)
7. [Movement & Warping](#7-movement--warping)
8. [Quests](#8-quests)
9. [Monsters & Mobs](#9-monsters--mobs)
10. [NPCs & Map](#10-npcs--map)
11. [Timers & Delays](#11-timers--delays)
12. [Party & Guild](#12-party--guild)
13. [Server & Announcements](#13-server--announcements)
14. [Advanced](#14-advanced)

---

## 1. DIALOGUE & DISPLAY

### mes "<message>"
Display a message in the dialog window.

```c
mes "[NPC Name]";
mes "Hello, adventurer!";
mes "Welcome to my shop.";
```

**Tips:**
- Use `[NPC Name]` as first line for name display
- Each `mes` is a new line in the dialog box
- Dialog displays until `next`, `close`, or `menu` command

---

### next
Display a "Next" button and wait for player to click it.

```c
mes "[Guide]";
mes "This is page 1.";
next;
mes "[Guide]";
mes "This is page 2.";
close;
```

---

### close / close2
Close the dialog window.

- `close` - Close and end script
- `close2` - Close but continue script execution

```c
mes "Goodbye!";
close;

// vs

mes "Processing...";
close2;
getitem 501,1;  // This still executes
end;
```

---

### menu "<option1>",<label>{,"<option2>",<label>...}
Display a menu with clickable options.

```c
mes "[Shop]";
mes "What do you need?";
menu
    "Red Potion",L_RedPotion,
    "Blue Potion",L_BluePotion,
    "Nothing",L_Cancel;

L_RedPotion:
    mes "Here's a Red Potion!";
    getitem 501,1;
    close;

L_BluePotion:
    mes "Here's a Blue Potion!";
    getitem 505,1;
    close;

L_Cancel:
    mes "Come back soon!";
    close;
```

---

### select("<option1>{:<option2>...}")
Modern menu syntax that returns selected option number.

```c
mes "[Shop]";
mes "What do you need?";
switch(select("Red Potion:Blue Potion:Yellow Potion:Cancel")) {
case 1:
    getitem 501,1;
    break;
case 2:
    getitem 505,1;
    break;
case 3:
    getitem 503,1;
    break;
case 4:
    mes "Goodbye!";
    break;
}
close;
```

**Advantage:** More compact than `menu`, easier to maintain.

---

### input {.<variable>}{,<min>{,<max>}}
Prompt player to enter a number or string.

```c
// Number input
mes "How many potions do you want?";
input .@amount,1,100;  // Min 1, Max 100
mes "You entered: " + .@amount;

// String input
mes "What's your name?";
input .@name$;
mes "Hello, " + .@name$ + "!";
```

---

### prompt("<message>"{,<min>,<max>})
Modern alternative to input with inline message.

```c
.@amount = prompt("How many potions?", 1, 100);
mes "You want " + .@amount + " potions.";
```

---

## 2. FLOW CONTROL

### if (<condition>) {<code>} {else {<code>}}
Conditional execution.

```c
if (BaseLevel >= 50) {
    mes "You are level 50+!";
} else if (BaseLevel >= 30) {
    mes "You are level 30-49.";
} else {
    mes "You are below level 30.";
}
```

**Comparison Operators:**
- `==` Equal to
- `!=` Not equal to
- `>` Greater than
- `<` Less than
- `>=` Greater than or equal
- `<=` Less than or equal
- `&&` AND
- `||` OR
- `!` NOT

---

### switch (<variable>) {case <value>: ... default: ...}
Multi-way branch.

```c
switch(Class) {
case Job_Swordman:
    mes "You are a Swordsman!";
    break;
case Job_Mage:
    mes "You are a Mage!";
    break;
case Job_Acolyte:
    mes "You are an Acolyte!";
    break;
default:
    mes "You are something else!";
    break;
}
```

---

### while (<condition>) {<code>}
Loop while condition is true.

```c
.@i = 0;
while (.@i < 10) {
    mes "Count: " + .@i;
    .@i++;
}
```

---

### for (<init>; <condition>; <increment>) {<code>}
For loop.

```c
for (.@i = 0; .@i < 10; .@i++) {
    mes "Count: " + .@i;
}
```

---

### do {<code>} while (<condition>)
Execute code at least once, then loop if condition true.

```c
do {
    mes "This runs at least once.";
} while (0);  // Won't loop
```

---

### break / continue
- `break` - Exit loop early
- `continue` - Skip to next iteration

```c
for (.@i = 0; .@i < 10; .@i++) {
    if (.@i == 5) continue;  // Skip 5
    if (.@i == 8) break;      // Stop at 8
    mes "Count: " + .@i;
}
```

---

### goto <label>
Jump to a label. **Use sparingly!**

```c
goto L_Skip;
mes "This won't show.";
L_Skip:
mes "Jumped here!";
```

---

### end
End script execution.

```c
mes "The end.";
end;
mes "This won't execute.";
```

---

## 3. VARIABLES & DATA

### Variable Types

```c
// Permanent character variables
MyVariable = 100;
MyString$ = "Hello";

// Temporary character variables
@TempVar = 100;
@TempString$ = "Temp";

// Permanent account variables
#AccountVar = 100;
#AccountString$ = "Account";

// Global permanent variables
$GlobalVar = 100;
$GlobalString$ = "Global";

// Global temporary variables
$@GlobalTemp = 100;

// NPC variables
.NPCVar = 100;
.NPCString$ = "NPC";

// Scope variables
.@ScopeVar = 100;
.@ScopeString$ = "Scope";

// Instance variables
'InstanceVar = 100;
```

**Scope Summary:**
- No prefix = Permanent character
- `@` = Temporary character
- `#` = Permanent account (this char server)
- `##` = Permanent account (all char servers)
- `$` = Permanent global
- `$@` = Temporary global
- `.` = NPC variable
- `.@` = Scope variable (function/script)
- `'` = Instance variable

---

### Arrays

```c
// Declare array
.@items[0] = 501;  // Red Potion
.@items[1] = 502;  // Orange Potion
.@items[2] = 503;  // Yellow Potion

// Loop through array
for (.@i = 0; .@i < getarraysize(.@items); .@i++) {
    mes "Item " + .@i + ": " + getitemname(.@items[.@i]);
}

// Get array size
.@size = getarraysize(.@items);

// Clear array
cleararray .@items[0],0,128;

// Copy array
copyarray .@dest[0], .@source[0], getarraysize(.@source);

// Delete array element
deletearray .@items[1],1;  // Delete index 1
```

---

### set <variable>,<value>
**Legacy** way to set variables. Use `=` instead.

```c
// Old way
set .@var, 100;

// New way (preferred)
.@var = 100;
```

---

### setarray <array>,<value>{,<value>...}
Set multiple array values at once.

```c
setarray .@items[0], 501, 502, 503, 504, 505;
```

---

### getarraysize(<array>)
Returns the size of an array.

```c
setarray .@items, 501, 502, 503;
.@size = getarraysize(.@items);  // Returns 3
```

---

### deletearray <array>{,<count>}
Delete array elements.

```c
deletearray .@items[0],1;  // Delete first element
deletearray .@items[0],getarraysize(.@items);  // Delete all
```

---

### cleararray <array>,<value>,<count>
Fill array with a value.

```c
cleararray .@items[0],0,10;  // Set first 10 elements to 0
```

---

## 4. PLAYER INFORMATION

### strcharinfo(<type>)
Get character information as string.

**Types:**
- `0` or `PC_NAME` - Character name
- `1` or `PC_PARTY` - Party name
- `2` or `PC_GUILD` - Guild name
- `3` or `PC_MAP` - Map name

```c
.@name$ = strcharinfo(PC_NAME);
.@party$ = strcharinfo(PC_PARTY);
.@guild$ = strcharinfo(PC_GUILD);
.@map$ = strcharinfo(PC_MAP);

mes "Your name is " + .@name$;
mes "Your party is " + .@party$;
```

---

### getcharid(<type>)
Get character ID numbers.

**Types:**
- `0` or `CHAR_ID_CHAR` - Character ID
- `1` or `CHAR_ID_PARTY` - Party ID
- `2` or `CHAR_ID_GUILD` - Guild ID
- `3` or `CHAR_ID_ACCOUNT` - Account ID
- `4` or `CHAR_ID_BG` - Battleground ID
- `5` or `CHAR_ID_CLAN` - Clan ID

```c
.@char_id = getcharid(CHAR_ID_CHAR);
.@account_id = getcharid(CHAR_ID_ACCOUNT);
.@party_id = getcharid(CHAR_ID_PARTY);
```

---

### Built-in Character Variables

```c
// Basic Stats
BaseLevel      // Base level
JobLevel       // Job level
BaseExp        // Current base exp
JobExp         // Current job exp
NextBaseExp    // Exp needed for next base level
NextJobExp     // Exp needed for next job level

// Resources
Zeny           // Current Zeny
Hp             // Current HP
MaxHp          // Maximum HP
Sp             // Current SP
MaxSp          // Maximum SP

// Status
StatusPoint    // Available status points
SkillPoint     // Available skill points
Weight         // Current weight
MaxWeight      // Maximum weight capacity

// Character Info
Class          // Job class ID
Sex            // 0=Female, 1=Male
Upper          // 0=Normal, 1=Advanced, 2=Baby

// Stats
Str, Agi, Vit, Int, Dex, Luk  // Base stats
```

**Example:**
```c
mes "Level: " + BaseLevel;
mes "Job: " + JobLevel;
mes "Zeny: " + Zeny;
mes "HP: " + Hp + "/" + MaxHp;

if (Zeny >= 1000) {
    Zeny -= 1000;
    mes "Paid 1000z.";
}
```

---

## 5. ITEMS & INVENTORY

### getitem <item_id>,<amount>{,<account_id>}
Give item(s) to player.

```c
// Give 10 Red Potions
getitem 501,10;

// Using item name constant
getitem Red_Potion,10;

// Give to specific player
getitem 501,10,getcharid(3);
```

---

### delitem <item_id>,<amount>
Remove item(s) from player.

```c
delitem 501,5;  // Remove 5 Red Potions
```

---

### countitem(<item_id>)
Count items in inventory.

```c
.@count = countitem(501);  // Count Red Potions
if (.@count >= 10) {
    mes "You have 10+ Red Potions!";
}
```

---

### checkweight(<item_id>,<amount>)
Check if player can carry item weight.

```c
if (checkweight(501,100)) {
    getitem 501,100;
} else {
    mes "You can't carry that much!";
}
```

---

### getitemname(<item_id>)
Get item name as string.

```c
.@name$ = getitemname(501);
mes "Item: " + .@name$;  // "Item: Red Potion"
```

---

### getnameditem(<item_id>,<player_name>)
Give item inscribed with player name.

```c
// Give sword inscribed with player's name
getnameditem 1101,strcharinfo(0);
```

---

## 6. PLAYER STATS & STATUS

### heal <hp>,<sp>
Heal player HP and/or SP.

```c
heal 100,50;      // Heal 100 HP and 50 SP
heal 1000,0;      // Heal 1000 HP only
heal 0,100;       // Heal 100 SP only
heal -50,0;       // Damage 50 HP
```

---

### percentheal <hp%>,<sp%>
Heal by percentage.

```c
percentheal 100,100;  // Full heal
percentheal 50,50;    // Heal 50% HP and SP
percentheal -10,0;    // Damage 10% HP
```

---

### statusup <stat>
Increase a stat by 1 point.

**Stats:** `bStr`, `bAgi`, `bVit`, `bInt`, `bDex`, `bLuk`

```c
statusup bStr;  // Add 1 STR
statusup bInt;  // Add 1 INT
```

---

### statusup2 <stat>,<amount>
Increase a stat by multiple points.

```c
statusup2 bStr,10;  // Add 10 STR
```

---

### readparam(<parameter>)
Read player parameters.

**Common Parameters:**
- `BaseLevel`, `JobLevel`
- `BaseExp`, `JobExp`
- `Zeny`
- `Hp`, `MaxHp`, `Sp`, `MaxSp`
- `bStr`, `bAgi`, `bVit`, `bInt`, `bDex`, `bLuk`

```c
.@level = readparam(BaseLevel);
.@str = readparam(bStr);
```

---

### sc_start <effect>,<duration>,<val1>{,<rate>,<flag>,<unit_id>}
Apply a status effect.

```c
// Blessing for 240 seconds (240000 ms), level 10
sc_start SC_BLESSING,240000,10;

// Increase AGI for 5 minutes, level 10
sc_start SC_INCREASEAGI,300000,10;

// Poison for 30 seconds
sc_start SC_POISON,30000,1;
```

**Common Status Effects:** See KB_REF_006 for full list.

---

### sc_end <effect>
Remove a status effect.

```c
sc_end SC_STONE;
sc_end SC_CURSE;
```

---

### skill <skill_id>,<level>{,<flag>}
Give player a skill.

```c
// Give Heal level 5
skill AL_HEAL,5;

// Temporary skill (doesn't save)
skill AL_HEAL,5,1;
```

---

## 7. MOVEMENT & WARPING

### warp "<map>",<x>,<y>
Warp player to location.

```c
warp "prontera",156,191;  // Warp to Prontera
warp "SavePoint",0,0;     // Warp to save point
warp "Random",0,0;        // Random location on current map
```

---

### areawarp "<from_map>",<x1>,<y1>,<x2>,<y2>,"<to_map>",<x>,<y>
Warp all players in an area.

```c
// Warp everyone in 10x10 area
areawarp "prontera",150,150,160,160,"payon",100,100;
```

---

### savepoint "<map>",<x>,<y>
Set player's save point.

```c
savepoint "prontera",156,191;
```

---

### return
Return to save point.

```c
mes "Returning you to save point!";
close2;
return;
end;
```

---

## 8. QUESTS

### setquest <quest_id>
Start a quest.

```c
setquest 1000;
mes "Quest started!";
```

---

### completequest <quest_id>
Complete a quest.

```c
if (questprogress(1000,HUNTING) == 2) {
    completequest 1000;
    mes "Quest completed!";
    getexp 10000,5000;
}
```

---

### erasequest <quest_id>
Remove a quest.

```c
erasequest 1000;
```

---

### questprogress(<quest_id>{,<type>})
Check quest status.

**Types:**
- `PLAYTIME` (0) - Check if time expired
- `HUNTING` (1) - Check if hunt complete
- `PLAYTIME|HUNTING` (2) - Check both

**Returns:**
- `0` - Quest not started
- `1` - Quest active
- `2` - Quest complete/expired

```c
.@status = questprogress(1000,HUNTING);
if (.@status == 0) {
    mes "Quest not started.";
} else if (.@status == 1) {
    mes "Quest in progress.";
} else if (.@status == 2) {
    mes "Quest objectives complete!";
}
```

---

## 9. MONSTERS & MOBS

### monster "<map>",<x>,<y>,"<name>",<mob_id>,<amount>{,<event>}
Spawn monster(s).

```c
// Spawn 1 Poring at coordinates
monster "prontera",156,191,"Poring",1002,1;

// Spawn 10 Porings randomly on map
monster "prontera",0,0,"Poring",1002,10;

// Spawn with death event
monster "prontera",0,0,"Poring",1002,5,"MyNPC::OnPoringKilled";
```

---

### areamonster "<map>",<x1>,<y1>,<x2>,<y2>,"<name>",<mob_id>,<amount>{,<event>}
Spawn monsters in an area.

```c
areamonster "prontera",150,150,160,160,"Poring",1002,10;
```

---

### killmonster "<map>","<event>"
Kill all monsters with event label.

```c
killmonster "prontera","All";  // Kill all
killmonster "prontera","MyNPC::OnKilled";  // Kill specific
```

---

### killmonsterall "<map>"
Kill all monsters on map.

```c
killmonsterall "prontera";
```

---

### getmobdrops(<mob_id>)
Get monster drop list.

```c
getmobdrops(1002);  // Get Poring drops
```

---

### getmonsterinfo(<mob_id>,<type>)
Get monster information.

**Types:**
- `MOB_NAME` - Name
- `MOB_LV` - Level
- `MOB_MAXHP` - Max HP
- `MOB_BASEEXP` - Base EXP
- `MOB_JOBEXP` - Job EXP
- `MOB_ATK1` - Attack 1
- `MOB_ATK2` - Attack 2
- `MOB_DEF` - Defense
- `MOB_MDEF` - Magic Defense
- `MOB_RACE` - Race
- `MOB_ELEMENT` - Element

```c
.@name$ = getmonsterinfo(1002,MOB_NAME);
.@hp = getmonsterinfo(1002,MOB_MAXHP);
```

---

## 10. NPCS & MAP

### enablenpc "<npc_name>"
Enable (show) an NPC.

```c
enablenpc "MyNPC";
```

---

### disablenpc "<npc_name>"
Disable (hide) an NPC.

```c
disablenpc "MyNPC";
```

---

### hideonnpc "<npc_name>"
Hide NPC (alternative).

```c
hideonnpc "MyNPC";
```

---

### hideoffnpc "<npc_name>"
Show NPC (alternative).

```c
hideoffnpc "MyNPC";
```

---

### donpcevent "<npc>::<label>"
Trigger an NPC event.

```c
donpcevent "MyNPC::OnEvent";
```

---

### doevent "<npc>::<label>"
Execute NPC event code.

```c
doevent "MyNPC::OnEvent";
```

---

### setmapflag "<map>",<mapflag>{,<val>}
Set a mapflag.

```c
setmapflag "prontera",mf_pvp;
setmapflag "prontera",mf_noteleport;
```

---

### removemapflag "<map>",<mapflag>
Remove a mapflag.

```c
removemapflag "prontera",mf_pvp;
```

---

### getmapflag("<map>",<mapflag>)
Check if mapflag is set.

```c
if (getmapflag("prontera",mf_pvp)) {
    mes "This is a PvP map!";
}
```

---

## 11. TIMERS & DELAYS

### sleep <milliseconds>
Pause script execution.

```c
mes "Wait 3 seconds...";
close2;
sleep 3000;
mes "Done waiting!";
end;
```

---

### sleep2 <milliseconds>
Sleep without blocking other scripts.

```c
sleep2 3000;
```

---

### addtimer <milliseconds>,"<npc>::<label>"
Set a timer.

```c
mes "See you in 10 seconds!";
close2;
addtimer 10000,"MyNPC::OnTimer";
end;

OnTimer:
    mes "Timer finished!";
    close;
```

---

### deltimer "<npc>::<label>"
Delete a timer.

```c
deltimer "MyNPC::OnTimer";
```

---

### initnpctimer {<flag>{,"<npc_name>"}
Start NPC timer.

```c
OnInit:
    initnpctimer;
    end;

OnTimer60000:  // Every 60 seconds
    announce "Server maintenance in 1 hour!",bc_all;
    stopnpctimer;
    end;
```

---

### stopnpctimer
Stop NPC timer.

```c
stopnpctimer;
```

---

## 12. PARTY & GUILD

### getpartymember <party_id>{,<type>}
Get party member list.

```c
getpartymember getcharid(CHAR_ID_PARTY);
for (.@i = 0; .@i < $@partymembercount; .@i++) {
    mes "Member: " + $@partymembername$[.@i];
}
```

---

### getguildmember <guild_id>{,<type>}
Get guild member list.

```c
getguildmember getcharid(CHAR_ID_GUILD);
```

---

### warpparty "<map>",<x>,<y>,<party_id>{,<from_map>}
Warp entire party.

```c
warpparty "prontera",156,191,getcharid(CHAR_ID_PARTY);
```

---

### warpguild "<map>",<x>,<y>,<guild_id>
Warp entire guild.

```c
warpguild "prontera",156,191,getcharid(CHAR_ID_GUILD);
```

---

## 13. SERVER & ANNOUNCEMENTS

### announce "<message>",<flag>{,<color>}
Server-wide announcement.

**Flags:**
- `bc_map` (0x01) - Map only
- `bc_area` (0x02) - Area only
- `bc_self` (0x04) - Self only
- `bc_all` (0x00) - All players
- `bc_yellow` (0x10) - Yellow text
- `bc_blue` (0x20) - Blue text
- `bc_woe` (0x40) - WoE format

```c
announce "Server maintenance in 10 minutes!",bc_all;
announce "You found a rare item!",bc_self;
announce "PvP event starting!",bc_map;
```

---

### mapannounce "<map>","<message>",<flag>
Map-only announcement.

```c
mapannounce "prontera","Event starting in Prontera!",bc_map;
```

---

### areaannounce "<map>",<x1>,<y1>,<x2>,<y2>,"<message>",<flag>
Area announcement.

```c
areaannounce "prontera",150,150,160,160,"Event here!",bc_area;
```

---

### dispbottom "<message>"
Display message at bottom of screen.

```c
dispbottom "You gained bonus experience!";
```

---

### atcommand "<command>"
Execute at-command.

```c
atcommand "@refresh";
atcommand "@heal";
```

---

## 14. ADVANCED

### callfunc("<function>"{,<arg>...})
Call a function.

```c
callfunc("MyFunction",100,"test");
```

---

### callsub <label>{,<arg>...}
Call a subroutine.

```c
callsub L_SubRoutine,100;
end;

L_SubRoutine:
    .@value = getarg(0);
    mes "Value: " + .@value;
    return;
```

---

### getarg(<index>{,<default>})
Get function/subroutine argument.

```c
function MyFunc {
    .@arg1 = getarg(0);
    .@arg2 = getarg(1,"default");
    return .@arg1 + .@arg2;
}
```

---

### setd / getd
Dynamic variable access.

```c
setd("$MyVar_" + .@id, 100);
.@value = getd("$MyVar_" + .@id);
```

---

### query_sql("<query>"{,<array>...})
Execute SQL query.

```c
query_sql("SELECT `name` FROM `char` WHERE `char_id` = " + .@char_id, .@name$);
```

---

### getusers(<type>)
Get player count.

**Types:**
- `0` - All online players
- `1` - Players on current map

```c
.@online = getusers(0);
mes "Players online: " + .@online;
```

---

### rand(<min>,<max>) / rand(<max>)
Generate random number.

```c
.@roll = rand(1,6);  // 1-6 (dice)
.@percent = rand(100);  // 0-99
```

---

### gettime(<type>)
Get current time/date.

**Types:**
- `DT_SECOND` (1) - Seconds (0-59)
- `DT_MINUTE` (2) - Minutes (0-59)
- `DT_HOUR` (3) - Hour (0-23)
- `DT_DAYOFWEEK` (4) - Day of week (0=Sunday)
- `DT_DAYOFMONTH` (5) - Day of month (1-31)
- `DT_MONTH` (6) - Month (1-12)
- `DT_YEAR` (7) - Year (4-digit)
- `DT_DAYOFYEAR` (8) - Day of year (1-366)

```c
.@hour = gettime(DT_HOUR);
if (.@hour >= 18 || .@hour < 6) {
    mes "It's nighttime!";
}
```

---

## COMMON PATTERNS

### Check Item Quest
```c
if (countitem(501) >= 10) {
    delitem 501,10;
    getexp 1000,500;
    mes "Quest complete!";
} else {
    mes "Need 10 Red Potions.";
}
```

### Level Check
```c
if (BaseLevel < 50) {
    mes "You need level 50+.";
    close;
}
```

### Zeny Cost
```c
if (Zeny < 1000) {
    mes "Not enough Zeny!";
    close;
}
Zeny -= 1000;
mes "Paid 1000z.";
```

### One-Time Event
```c
if (#STARTER_RECEIVED) {
    mes "You already got this.";
    close;
}
getitem 501,10;
set #STARTER_RECEIVED,1;
mes "Here's your starter pack!";
```

### Cooldown Timer
```c
if (#LAST_USE + 3600 > gettimetick(2)) {
    mes "Come back in " + ((#LAST_USE + 3600 - gettimetick(2)) / 60) + " minutes.";
    close;
}
set #LAST_USE,gettimetick(2);
mes "Ready to use!";
```

---

## DEBUGGING

### debugmes "<message>"
Output to console (server-side only).

```c
debugmes "Debug: Variable = " + .@var;
```

---

### dispbottom "<message>"
Show message to player.

```c
dispbottom "Debug: HP = " + Hp;
```

---

## SEE ALSO

- **Full Reference:** `/doc/script_commands.txt` (11,721 lines, 400+ commands)
- **KB_EXAMPLE_001:** NPC Script Examples
- **KB_REF_006:** Status Effects Reference
- **KB_REF_007:** Visual Effects Reference
- **KB_REF_008:** Item Bonus Reference

---

*Last Updated: 2025-10-23*
*rAthena Documentation - Essential Commands*
