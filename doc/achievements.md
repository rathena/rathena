<!--
//===== rAthena Documentation ================================
//= Achievement Database Structure
//===== By: ==================================================
//= rAthena Dev Team
//===== Last Updated: ========================================
//= 20200220
//===== Description: =========================================
//= Explanation of the achievements_db.yml file and structure.
//============================================================
-->

# Achievement Database Structure

### Id
Unique achievement ID.

### Group
Achievement group type.  
Each achievement type calls a specific objective check.  
<details class="card">
  <summary>Valid groups</summary>
  <div class="card-body">
    <ul>
      <li><code>None</code> - Can be used for custom achievements that are given through a script with no trigger events.</li>
      <li><code>Add_Friend</code> - Triggered when a player adds a friend.</li>
      <li><code>Adventure</code> - Does not trigger automatically. These are triggered by the achievementcomplete script command.</li>
      <li><code>Baby</code> - Triggered when a player becomes a baby job.</li>
      <li><code>Battle</code> - Triggered when a player kills a monster.</li>
      <li><code>Chatting</code> - Aegis uses this when talking to a NPC. These are triggered by the achievementupdate script command.</li>
      <li><code>Chatting_Count</code> - Triggered when a player has a chatroom open and others join.</li>
      <li><code>Chatting_Create</code> - Triggered when a player creates a chatroom.</li>
      <li><code>Chatting_Dying</code> - Triggered when a player creates a chatroom and dies with it open.</li>
      <li><code>Eat</code> - Unknown.</li>
      <li><code>Get_Item</code> - Triggered when a player gets an item that has a specific sell value.</li>
      <li><code>Get_Zeny</code> - Triggered when a player gets a specific amount of zeny at once.</li>
      <li><code>Goal_Achieve</code> - Triggered when a player's achievement rank levels up.</li>
      <li><code>Goal_Level</code> - Triggered when a player's base level or job level changes.</li>
      <li><code>Goal_Status</code> - Triggered when a player's base stats changes.</li>
      <li><code>Job_Change</code> - Triggered when a player's job changes.</li>
      <li><code>Marry</code> - Triggered when two players get married.</li>
      <li><code>Party</code> - Triggered when a player creates a party.</li>
      <li><code>Enchant_Fail</code> - Triggered when a player fails to refine an equipment.</li>
      <li><code>Enchant_Success</code> - Triggered when a player successfully refines an equipment.</li>
      <li><code>Spend_Zeny</code> - Triggered when a player spends any amount of zeny on vendors.</li>
      <li><code>Taming</code> - Triggered when a player tames a monster.</li>
    </ul>
  </div>
</details>


### Name
Achievement name. Used when sending rewards through RODEX.

### Targets
A list of monster names and count values that the achievement requires. The target count can also be used for achievements that keep a counter while not being related to monster kills. Capped at `MAX_ACHIEVEMENT_OBJECTIVES`. See examples below:

Player must kill 5 Scorpions and 10 Porings
```yml
  Targets:
    - Id: 0
      Mob: SCORPION
      Count: 5
    - Id: 1
      Mob: PORING
      Count: 10
```

Player must have 100 or more of ARG0 value. Using the count target value is useful for achievements that are increased in increments and not checked for a total (UI_Type = 1). IE: In the achievement_list.lub file, UI_Type 0 is displayed as non-incremental while 1 shows a progress bar of completion for the achievement.
```yml
Condition: " ARG0 >= 100 "
  Targets:
    - Id: 0 // Array index value
      Count: 100
```

### Condition
A conditional statement that must be met for the achievement to be considered complete. Accepts script constants, player variables, and ARGX (where X is the argument vector value). The ARGX values are sent from the server to the achievement script engine on special events. See examples below:

This function will send 1 argument (ARG0) with a value of i + 1 when a friend is added.
```yml
achievement_update_objective(f_sd, AG_ADD_FRIEND, 1, i + 1);
```

This function will send 2 arguments (ARG0 and ARG1) with values of weapon level and refine level, respectively, when an equipment is successfully refined.
```yml
achievement_update_objective(sd, AG_REFINE_SUCCESS, 2, sd->inventory_data[i]->wlv, sd->inventory.u.items_inventory[i].refine);
```

### Map
A map name that is used for the Chatting group which increments the counter based on the player's map.  
> NOTICE: This option is currently disabled until the official behavior is confirmed.

### Dependents
A list of achievement IDs that need to be completed before this achievement is considered complete. See examples below:

Player must complete achievements 10001 and 10002 first.
```yml
Dependents:
  10001: true
  10002: true
```

Used with the import, dependent achievements can be disabled. The player now only requires completion of achievement 10001.
```
Dependents:
  10002: false
```

### Rewards
A list of rewards that are given on completion. All fields are optional.
```yml
Item: Item Name
Amount: Amount of Item (Default: 1)
Script: Bonus Script
TitleId: Title ID
```

### Score
Achievement points that are given on completion.

## Example
```yml
 - Id: 99                  
   Group: Baby            
   Name: Example Achieve
   Targets:            
     - Id: 0
       Mob: XM_CELINE_KIMI
       Count: 1
   Condition: " BaseLevel >= 99 "
   Map: prontera
   Dependents:
     - Id: 100
   Rewards:
     Item: Shabby_Purse
     Amount: 10
     Script: " specialeffect2 EF_BLESSING; sc_start SC_BLESSING,30000,10; "
     TitleId: 1000
   Score: 10
```
