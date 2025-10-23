---
kb_id: KB_REF_001
kb_type: reference
kb_category: permissions
kb_subcategory: player_groups
kb_keywords: [permissions, groups, access control, player rights, admin, command access, PC_PERM, groups.conf, can_trade, all_commands, bypass, security]
kb_related: [KB_CONF_002]
kb_difficulty: intermediate
kb_version: rAthena_2025
kb_last_updated: 2024-04-14
kb_use_case: [server_configuration, admin_setup, permission_management]
---

# rAthena Permission System Reference

Complete reference for player group permissions configured in `/conf/groups.conf`.

## Overview

Permissions control what players in specific groups can and cannot do on your server. Each permission can be enabled or disabled for different player groups (e.g., normal players, moderators, administrators).

**Configuration Location:** `/conf/groups.conf`

**Format in groups.conf:**
```yaml
permissions:
  permission_name: true/false
```

**Constant Name Format:** Used in scripts and source code as `PC_PERM_CONSTANT_NAME`

---

## 1. BASIC PERMISSIONS

### can_trade (PC_PERM_TRADE)
**Description:** Allows player to distribute items through various means.
**Affects:**
- Trading with other players
- Dropping items on ground
- Vending (selling via shop)
- Storage access (put/get items)
- Mail system
- Any item distribution method

**Use Case:** Disable for trial accounts or restricted players to prevent item trading.

---

### can_party (PC_PERM_PARTY)
**Description:** Allows player to create and join parties.
**Affects:**
- Creating new parties
- Joining existing parties
- Party invitations

**Use Case:** Restrict party functionality for specific player groups.

---

### attendance (PC_PERM_ATTENDANCE)
**Description:** Allows player to use the daily attendance system.
**Affects:**
- Daily login rewards
- Attendance tracking

**Use Case:** Enable/disable daily reward system access.

---

## 2. EXTENDED PERMISSIONS

### all_skill (PC_PERM_ALL_SKILL)
**Description:** Grants player ALL available skills in their skill tree.
**Affects:**
- Automatically grants all skills
- Bypasses normal skill learning requirements

**Use Case:** Testing, GM characters, special events.

**Warning:** This is a powerful permission. Use carefully.

---

### all_equipment (PC_PERM_USE_ALL_EQUIPMENT)
**Description:** Allows player to equip ANY item regardless of requirements.
**Bypasses:**
- Class restrictions
- Level requirements
- Gender restrictions
- All equipment restrictions

**Use Case:** Testing, GM characters.

**Warning:** May cause client errors if sprite doesn't exist for the player's class.

---

### skill_unconditional (PC_PERM_SKILL_UNCONDITIONAL)
**Description:** Allows player to use ANY skill without meeting conditions.
**Bypasses:**
- SP cost requirements
- Item requirements (catalysts, ammunition)
- Cooldown times
- All skill usage conditions

**Use Case:** GM characters, testing, special events.

---

### join_chat (PC_PERM_JOIN_ALL_CHAT)
**Description:** Allows player to join password-protected chatrooms.
**Affects:**
- Can enter any chatroom without password

**Use Case:** Moderator oversight, GM monitoring.

---

### kick_chat (PC_PERM_NO_CHAT_KICK)
**Description:** Prevents player from being kicked from chatrooms.
**Affects:**
- Immunity to chatroom kicks

**Use Case:** GM/moderator protection.

---

### view_hpmeter (PC_PERM_VIEW_HPMETER)
**Description:** Allows player to see HP bar of EVERY player.
**Affects:**
- Can view all player HP bars
- Overrides individual player settings

**Use Case:** GM monitoring, debugging.

---

### view_equipment (PC_PERM_VIEW_EQUIPMENT)
**Description:** Allows player to view equipment of EVERY player.
**Bypasses:**
- Individual player privacy settings
- Equipment viewing restrictions

**Use Case:** GM inspection, player support.

---

### hack_info (PC_PERM_RECEIVE_HACK_INFO)
**Description:** Allows player to receive all information about hacking attempts.
**Receives:**
- Name spoofing attempts
- Hack attempt notifications
- Security breach alerts

**Use Case:** Security team, administrators.

---

### disable_pvm (PC_PERM_DISABLE_PVM)
**Description:** PREVENTS player from attacking monsters.
**Affects:**
- Cannot engage in PvM (Player vs Monster) combat

**Use Case:** Event NPCs, special restricted accounts, RP-only characters.

---

### disable_pvp (PC_PERM_DISABLE_PVP)
**Description:** PREVENTS player from attacking other players.
**Affects:**
- Cannot engage in PvP (Player vs Player) combat

**Use Case:** Protected accounts, non-combat roles, moderators observing.

---

### can_trade_bounded (PC_PERM_TRADE_BOUNDED)
**Description:** Allows player to perform normal item actions with bounded items.
**Enables:**
- Dropping bound items
- Selling bound items
- Trading bound items
- All item actions with bound items

**Use Case:** GM item management, special accounts.

**Note:** Bounded items are normally account/character-locked.

---

### item_unconditional (PC_PERM_ITEM_UNCONDITIONAL)
**Description:** Allows player to consume ANY consumable item without requirements.
**Bypasses:**
- noitemconsumption mapflag
- Item's class restrictions
- Gender restrictions
- Status change requirements
- Item delay/cooldown
- All consumption requirements

**Use Case:** Testing, GM characters, debugging.

---

### trade_unconditional (PC_PERM_TRADE_UNCONDITIONAL)
**Description:** Allows player to ignore ALL trade conditions of items.
**Bypasses:**
- No-drop restrictions
- No-trade restrictions
- No-sell restrictions
- Cart restrictions
- Storage/guild storage restrictions
- Mail restrictions
- Auction restrictions

**Use Case:** GM item management, special administrative tasks.

---

## 3. COMMAND-RELATED PERMISSIONS

### all_commands (PC_PERM_USE_ALL_COMMANDS)
**Description:** Allows usage of ALL atcommands and charcommands.
**Grants:**
- Full access to all @ commands
- Full access to all # commands

**Use Case:** Full administrators.

**Warning:** This is the highest privilege level.

---

### disable_commands_when_dead (PC_PERM_DISABLE_CMD_DEAD)
**Description:** DISABLES usage of atcommands when player is dead.
**Affects:**
- Cannot use commands while dead

**Use Case:** Enforce fair play, prevent abuse.

---

### hide_session (PC_PERM_HIDE_SESSION)
**Description:** Hides player session from being displayed by atcommands.
**Hides From:**
- @who command
- @whomap command
- @whogm command
- Other player-listing commands

**Use Case:** GM anonymity, undercover monitoring.

---

### who_display_aid (PC_PERM_WHO_DISPLAY_AID)
**Description:** Displays all GMs and character/account IDs in @who command.
**Shows:**
- All GM accounts (even hidden)
- Character IDs
- Account IDs

**Use Case:** Administrator oversight, debugging.

---

### any_warp (PC_PERM_WARP_ANYWHERE)
**Description:** Allows player to bypass warp-related mapflags.
**Bypasses:**
- nowarp mapflag
- nowarpto mapflag
- noteleport mapflag
- nomemo mapflag

**Affected Commands:**
- @memo
- @mapmove
- @go
- @jump
- @warp
- All warp commands

**Use Case:** GM mobility, administrative access.

---

### receive_requests (PC_PERM_RECEIVE_REQUESTS)
**Description:** Allows player to receive requests through @requests command.
**Affects:**
- Receives player requests/reports

**Use Case:** Support staff, moderators.

---

### show_bossmobs (PC_PERM_SHOW_BOSS)
**Description:** Displays boss mobs in @showmobs command.
**Shows:**
- MVP/boss locations
- Boss mob information

**Use Case:** GM hunting, event management, debugging.

---

### channel_admin (PC_PERM_CHANNEL_ADMIN)
**Description:** Allows player to modify #channel settings regardless of ownership.
**Grants:**
- Full channel administration
- Modify any channel settings
- Join password-protected channels without password
- Override channel ownership

**Use Case:** Chat moderators, administrators.

---

### use_check (PC_PERM_USE_CHECK)
**Description:** Allows player to use client command /check.
**Enables:**
- /check command (displays character status)

**Use Case:** GM inspection, debugging.

---

### use_changemaptype (PC_PERM_USE_CHANGEMAPTYPE)
**Description:** Allows player to use client command /changemaptype.
**Enables:**
- /changemaptype command

**Use Case:** Testing, special effects.

---

### command_enable (PC_PERM_ENABLE_COMMAND)
**Description:** Enable use of atcommands while talking with NPC.
**Allows:**
- Using @ commands during NPC dialogue

**Use Case:** GM convenience, debugging NPC scripts.

---

### bypass_stat_onclone (PC_PERM_BYPASS_STAT_ONCLONE)
**Description:** Bypass max parameter limit while using @clonestat.
**Bypasses:**
- Maximum stat limits when cloning stats

**Use Case:** GM testing, special events.

---

### bypass_max_stat (PC_PERM_BYPASS_MAX_STAT)
**Description:** Allows bypassing maximum stat parameter to absolute maximum (32,767).
**Bypasses:**
- conf/player.conf stat limits
- Normal stat caps

**Maximum Value:** 32,767

**Use Case:** Testing extreme cases, special characters.

**Warning:** Can severely imbalance gameplay.

---

### macro_detect (PC_PERM_MACRO_DETECT)
**Description:** Allows player to use client command /macro_detector.
**Enables:**
- /macro_detector command
- Bot detection tools

**Use Case:** GM bot hunting, anti-cheat operations.

---

### macro_register (PC_PERM_MACRO_REGISTER)
**Description:** Allows player to use captcha management commands.
**Enables:**
- /macro_register (add new captcha)
- /macro_preview (preview captcha by ID)

**Use Case:** Captcha system management, anti-bot configuration.

---

## QUICK REFERENCE TABLE

| Permission Name | Constant | Type | Purpose |
|----------------|----------|------|---------|
| can_trade | PC_PERM_TRADE | Basic | Allow item distribution |
| can_party | PC_PERM_PARTY | Basic | Allow party system |
| attendance | PC_PERM_ATTENDANCE | Basic | Allow daily attendance |
| all_skill | PC_PERM_ALL_SKILL | Extended | Grant all skills |
| all_equipment | PC_PERM_USE_ALL_EQUIPMENT | Extended | Equip any item |
| skill_unconditional | PC_PERM_SKILL_UNCONDITIONAL | Extended | Use skills without conditions |
| join_chat | PC_PERM_JOIN_ALL_CHAT | Extended | Join protected chatrooms |
| kick_chat | PC_PERM_NO_CHAT_KICK | Extended | Immune to chatroom kicks |
| view_hpmeter | PC_PERM_VIEW_HPMETER | Extended | See all player HP |
| view_equipment | PC_PERM_VIEW_EQUIPMENT | Extended | See all player equipment |
| hack_info | PC_PERM_RECEIVE_HACK_INFO | Extended | Receive hack alerts |
| disable_pvm | PC_PERM_DISABLE_PVM | Extended | Prevent PvM combat |
| disable_pvp | PC_PERM_DISABLE_PVP | Extended | Prevent PvP combat |
| can_trade_bounded | PC_PERM_TRADE_BOUNDED | Extended | Trade bound items |
| item_unconditional | PC_PERM_ITEM_UNCONDITIONAL | Extended | Use items unconditionally |
| trade_unconditional | PC_PERM_TRADE_UNCONDITIONAL | Extended | Ignore trade restrictions |
| all_commands | PC_PERM_USE_ALL_COMMANDS | Command | Use all commands |
| disable_commands_when_dead | PC_PERM_DISABLE_CMD_DEAD | Command | Disable commands when dead |
| hide_session | PC_PERM_HIDE_SESSION | Command | Hide from @who |
| who_display_aid | PC_PERM_WHO_DISPLAY_AID | Command | Show IDs in @who |
| any_warp | PC_PERM_WARP_ANYWHERE | Command | Bypass warp restrictions |
| receive_requests | PC_PERM_RECEIVE_REQUESTS | Command | Receive player requests |
| show_bossmobs | PC_PERM_SHOW_BOSS | Command | Show bosses in @showmobs |
| channel_admin | PC_PERM_CHANNEL_ADMIN | Command | Full channel control |
| use_check | PC_PERM_USE_CHECK | Command | Use /check command |
| use_changemaptype | PC_PERM_USE_CHANGEMAPTYPE | Command | Use /changemaptype |
| command_enable | PC_PERM_ENABLE_COMMAND | Command | Commands during NPC talk |
| bypass_stat_onclone | PC_PERM_BYPASS_STAT_ONCLONE | Command | Bypass @clonestat limits |
| bypass_max_stat | PC_PERM_BYPASS_MAX_STAT | Command | Max stat = 32,767 |
| macro_detect | PC_PERM_MACRO_DETECT | Command | Use /macro_detector |
| macro_register | PC_PERM_MACRO_REGISTER | Command | Manage captchas |

---

## USAGE EXAMPLES

### Example 1: Normal Player Group
```yaml
groups:
  - Name: Player
    Level: 0
    permissions:
      can_trade: true
      can_party: true
      attendance: true
```

### Example 2: Trial Account
```yaml
groups:
  - Name: Trial
    Level: 0
    permissions:
      can_trade: false        # Prevent trading
      can_party: true
      attendance: false
```

### Example 3: Moderator
```yaml
groups:
  - Name: Moderator
    Level: 50
    permissions:
      can_trade: true
      can_party: true
      view_hpmeter: true
      view_equipment: true
      hack_info: true
      receive_requests: true
      channel_admin: true
      hide_session: true
```

### Example 4: Administrator
```yaml
groups:
  - Name: Admin
    Level: 99
    permissions:
      all_commands: true
      any_warp: true
      skill_unconditional: true
      all_equipment: true
      bypass_max_stat: true
      view_hpmeter: true
      view_equipment: true
```

---

## BEST PRACTICES

1. **Least Privilege Principle:** Only grant permissions that are absolutely necessary
2. **Group Hierarchy:** Create multiple group levels (Player → Helper → Moderator → Admin)
3. **Testing:** Test permissions thoroughly in a development environment
4. **Documentation:** Document your custom group configurations
5. **Security:** Limit `all_commands` and `all_equipment` to trusted administrators only
6. **Monitoring:** Use `hack_info` and `receive_requests` for security team

---

## RELATED CONFIGURATION FILES

- `/conf/groups.yml` - Main permission configuration
- `/conf/atcommands.yml` - Command-specific permissions
- `/doc/atcommands.txt` - Available commands documentation

---

## SEE ALSO

- **KB_REF_002:** Admin Commands Reference
- **KB_CONF_001:** Configuration System Guide
- **KB_CONF_002:** Groups.yml Configuration

---

*Last Updated: 2024-04-14*
*rAthena Documentation*
