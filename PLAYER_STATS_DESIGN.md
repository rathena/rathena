# การออกแบบระบบเก็บสถิติผู้เล่น (Player Statistics Storage System)

## ภาพรวม (Overview)

ระบบนี้จะเก็บสถิติการเล่นของผู้เล่นแบบ Real-time เพื่อใช้ในการวิเคราะห์พฤติกรรมผู้เล่น, Achievement System, และ Ranking

## 1. โครงสร้างฐานข้อมูล (Database Schema)

### ตาราง: `player_statistics`

```sql
CREATE TABLE IF NOT EXISTS `player_statistics` (
  `char_id` int(11) unsigned NOT NULL,
  `account_id` int(11) unsigned NOT NULL,

  -- เวลาออนไลน์ (Online Time Tracking)
  `online_time` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total online time in seconds',
  `session_start` datetime DEFAULT NULL COMMENT 'Current session start time',
  `last_login` datetime DEFAULT NULL COMMENT 'Last login timestamp',
  `login_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Total number of logins',

  -- การใช้ไอเทม (Item Usage)
  `item_used_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total items used/consumed',
  `item_used_healing` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Healing items used',
  `item_used_buff` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Buff items used',
  `item_used_other` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Other items used',

  -- การเคลื่อนย้าย (Movement & Teleport)
  `teleport_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Teleport skill uses',
  `warp_portal_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Warp portal uses',
  `butterfly_wing_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Butterfly Wing uses',
  `movement_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total cell movements',
  `movement_distance` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total distance in cells',

  -- การรักษา HP/SP (Healing & SP Recovery)
  `heal_skill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Heal skill uses',
  `heal_amount_total` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total HP healed',
  `sp_recovery_skill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'SP recovery skill uses',
  `sp_recovery_amount` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total SP recovered',

  -- การเก็บไอเทม (Item Pickup)
  `item_picked_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total items picked up',
  `item_picked_from_mob` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Items from monsters',
  `item_picked_from_ground` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Items from ground',
  `zeny_picked` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total zeny picked',

  -- การล่ามอนสเตอร์ (Monster Kills)
  `mob_kill_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total monsters killed',
  `mvp_kill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'MVP monsters killed',
  `boss_kill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Boss monsters killed',
  `mini_boss_kill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Mini-boss monsters killed',

  -- การใช้สกิล (Skill Usage)
  `skill_used_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total skills used',
  `skill_offensive_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Offensive skills',
  `skill_support_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Support skills',
  `skill_passive_triggered` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Passive skill triggers',

  -- สถิติเพิ่มเติม (Additional Stats)
  `death_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Total deaths',
  `damage_dealt` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total damage dealt',
  `damage_received` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total damage received',
  `chat_message_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Chat messages sent',

  -- วันที่อัปเดต (Update Timestamps)
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

  PRIMARY KEY (`char_id`),
  KEY `account_id` (`account_id`),
  KEY `online_time` (`online_time`),
  KEY `mob_kill_count` (`mob_kill_count`),
  KEY `updated_at` (`updated_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

### ตาราง: `player_statistics_daily` (สำหรับสถิติรายวัน)

```sql
CREATE TABLE IF NOT EXISTS `player_statistics_daily` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) unsigned NOT NULL,
  `stat_date` date NOT NULL COMMENT 'Statistics date',

  -- สถิติรายวัน (Daily Statistics)
  `online_time` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Daily online time in seconds',
  `item_used` int(11) unsigned NOT NULL DEFAULT '0',
  `teleport_count` int(11) unsigned NOT NULL DEFAULT '0',
  `movement_count` int(11) unsigned NOT NULL DEFAULT '0',
  `heal_count` int(11) unsigned NOT NULL DEFAULT '0',
  `item_picked` int(11) unsigned NOT NULL DEFAULT '0',
  `mob_kill` int(11) unsigned NOT NULL DEFAULT '0',
  `skill_used` int(11) unsigned NOT NULL DEFAULT '0',
  `exp_gained` bigint(20) unsigned NOT NULL DEFAULT '0',
  `zeny_gained` bigint(20) unsigned NOT NULL DEFAULT '0',

  PRIMARY KEY (`id`),
  UNIQUE KEY `char_date` (`char_id`, `stat_date`),
  KEY `stat_date` (`stat_date`),
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

### ตาราง: `player_statistics_detail` (สำหรับข้อมูลละเอียด)

```sql
CREATE TABLE IF NOT EXISTS `player_statistics_detail` (
  `char_id` int(11) unsigned NOT NULL,

  -- Top 10 Skills Used (JSON)
  `top_skills_used` TEXT DEFAULT NULL COMMENT 'JSON: {skill_id: count}',

  -- Top 10 Items Used (JSON)
  `top_items_used` TEXT DEFAULT NULL COMMENT 'JSON: {item_id: count}',

  -- Top 10 Monsters Killed (JSON)
  `top_mobs_killed` TEXT DEFAULT NULL COMMENT 'JSON: {mob_id: count}',

  -- Map Statistics (JSON)
  `map_visit_count` TEXT DEFAULT NULL COMMENT 'JSON: {map_name: count}',
  `map_time_spent` TEXT DEFAULT NULL COMMENT 'JSON: {map_name: seconds}',

  -- Session History (JSON array of last 30 sessions)
  `session_history` TEXT DEFAULT NULL COMMENT 'JSON: [{start, end, duration}]',

  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

  PRIMARY KEY (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

---

## 2. โครงสร้างข้อมูลใน C++ (Data Structures)

### ไฟล์: `src/map/player_statistics.hpp` (ใหม่)

```cpp
#ifndef PLAYER_STATISTICS_HPP
#define PLAYER_STATISTICS_HPP

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

/**
 * Player Statistics Structure
 * Stores real-time player activity statistics
 */
struct player_statistics {
    uint32 char_id;
    uint32 account_id;

    // Online Time Tracking
    uint64 online_time;           // Total online time in seconds
    time_t session_start;         // Current session start timestamp
    time_t last_login;            // Last login timestamp
    uint32 login_count;           // Total number of logins

    // Item Usage
    uint64 item_used_count;       // Total items used
    uint32 item_used_healing;     // Healing items
    uint32 item_used_buff;        // Buff items
    uint32 item_used_other;       // Other items

    // Movement & Teleport
    uint32 teleport_count;        // Teleport skill uses
    uint32 warp_portal_count;     // Warp portal uses
    uint32 butterfly_wing_count;  // Butterfly Wing uses
    uint64 movement_count;        // Total cell movements
    uint64 movement_distance;     // Total distance in cells

    // Healing & SP Recovery
    uint32 heal_skill_count;      // Heal skill uses
    uint64 heal_amount_total;     // Total HP healed
    uint32 sp_recovery_skill_count; // SP recovery skill uses
    uint64 sp_recovery_amount;    // Total SP recovered

    // Item Pickup
    uint64 item_picked_count;     // Total items picked
    uint32 item_picked_from_mob;  // Items from monsters
    uint32 item_picked_from_ground; // Items from ground
    uint64 zeny_picked;           // Total zeny picked

    // Monster Kills
    uint64 mob_kill_count;        // Total monsters killed
    uint32 mvp_kill_count;        // MVP monsters killed
    uint32 boss_kill_count;       // Boss monsters killed
    uint32 mini_boss_kill_count;  // Mini-boss monsters killed

    // Skill Usage
    uint64 skill_used_count;      // Total skills used
    uint32 skill_offensive_count; // Offensive skills
    uint32 skill_support_count;   // Support skills
    uint32 skill_passive_triggered; // Passive skill triggers

    // Additional Stats
    uint32 death_count;           // Total deaths
    uint64 damage_dealt;          // Total damage dealt
    uint64 damage_received;       // Total damage received
    uint32 chat_message_count;    // Chat messages sent

    // Flags
    bool dirty;                   // Needs to be saved
};

// Function declarations
void player_statistics_init(void);
void player_statistics_final(void);

struct player_statistics* player_statistics_load(uint32 char_id);
void player_statistics_save(struct player_statistics* stats, bool force = false);
void player_statistics_create(uint32 char_id, uint32 account_id);
void player_statistics_delete(uint32 char_id);

// Tracking functions
void player_statistics_track_login(struct map_session_data* sd);
void player_statistics_track_logout(struct map_session_data* sd);
void player_statistics_track_item_use(struct map_session_data* sd, struct item* it, int type);
void player_statistics_track_teleport(struct map_session_data* sd, int type);
void player_statistics_track_movement(struct map_session_data* sd, int x0, int y0, int x1, int y1);
void player_statistics_track_heal(struct map_session_data* sd, int hp, int sp);
void player_statistics_track_item_pickup(struct map_session_data* sd, struct item* it, int amount, bool from_mob);
void player_statistics_track_mob_kill(struct map_session_data* sd, struct mob_data* md);
void player_statistics_track_skill_use(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv, int type);
void player_statistics_track_death(struct map_session_data* sd);
void player_statistics_track_damage(struct map_session_data* sd, int64 damage, bool dealt);
void player_statistics_track_chat(struct map_session_data* sd);

// Update functions
void player_statistics_update_online_time(struct map_session_data* sd);
void player_statistics_autosave_timer(int tid, int64 tick, int id, intptr_t data);

// Query functions
uint64 player_statistics_get_online_time(struct map_session_data* sd);
uint64 player_statistics_get_stat(uint32 char_id, const char* stat_name);

#endif /* PLAYER_STATISTICS_HPP */
```

### ไฟล์: `src/common/mmo.hpp` (เพิ่มเข้าไปใน struct map_session_data)

```cpp
// ใน struct map_session_data เพิ่ม:
struct player_statistics* statistics; // Player statistics data
```

---

## 3. การเพิ่มใน Schema Configuration

### ไฟล์: `src/char/char.hpp` (เพิ่มชื่อตารางใน Schema_Config)

```cpp
struct Schema_Config {
    // ... existing tables ...

    // Player Statistics Tables
    std::string player_statistics_table;
    std::string player_statistics_daily_table;
    std::string player_statistics_detail_table;
}
```

### ไฟล์: `conf/char_athena.conf` (เพิ่ม configuration)

```yaml
// Player Statistics Tables
player_statistics_db: player_statistics
player_statistics_daily_db: player_statistics_daily
player_statistics_detail_db: player_statistics_detail

// Auto-save interval for statistics (in seconds, default: 300 = 5 minutes)
player_statistics_autosave_interval: 300

// Enable detailed statistics tracking (skill details, item details, etc.)
player_statistics_detailed: true
```

---

## 4. จุดเชื่อมต่อในโค้ด (Integration Points)

### 4.1 การ Login/Logout

**ไฟล์**: `src/map/pc.cpp`

```cpp
// ใน function: pc_authok()
// หลังจากโหลดข้อมูลผู้เล่นสำเร็จ
sd->statistics = player_statistics_load(sd->status.char_id);
if (!sd->statistics) {
    player_statistics_create(sd->status.char_id, sd->status.account_id);
    sd->statistics = player_statistics_load(sd->status.char_id);
}
player_statistics_track_login(sd);

// ใน function: pc_logout() หรือ map_quit()
player_statistics_track_logout(sd);
player_statistics_save(sd->statistics, true); // Force save on logout
```

### 4.2 การใช้ไอเทม

**ไฟล์**: `src/map/pc.cpp` - function `pc_useitem()`

```cpp
// หลังจากใช้ไอเทมสำเร็จ
int item_type = 0; // 0=other, 1=healing, 2=buff
if (id->type == IT_HEALING) item_type = 1;
else if (id->type == IT_CASH || /* check buff items */) item_type = 2;
player_statistics_track_item_use(sd, &sd->inventory.u.items_inventory[n], item_type);
```

### 4.3 การ Teleport

**ไฟล์**: `src/map/skill.cpp` - function `skill_castend_nodamage_id()`

```cpp
case AL_TELEPORT:
case TK_HIGHJUMP:
    // หลังจาก teleport สำเร็จ
    player_statistics_track_teleport(sd, 0); // 0=skill
    break;

// ใน pc_useitem() สำหรับ Butterfly Wing / Fly Wing
if (nameid == ITEMID_FLY_WING || nameid == ITEMID_BUTTERFLY_WING) {
    player_statistics_track_teleport(sd, nameid == ITEMID_BUTTERFLY_WING ? 2 : 1);
}
```

### 4.4 การเคลื่อนไหว

**ไฟล์**: `src/map/unit.cpp` - function `unit_walktoxy()`

```cpp
// หลังจากเริ่มเดิน
if (bl->type == BL_PC) {
    struct map_session_data* sd = BL_CAST(BL_PC, bl);
    player_statistics_track_movement(sd, bl->x, bl->y, x, y);
}
```

### 4.5 การรักษา HP/SP

**ไฟล์**: `src/map/battle.cpp` - function `battle_heal()`

```cpp
if (hp > 0 || sp > 0) {
    if (bl->type == BL_PC) {
        struct map_session_data* sd = BL_CAST(BL_PC, bl);
        player_statistics_track_heal(sd, hp, sp);
    }
}
```

### 4.6 การเก็บไอเทม

**ไฟล์**: `src/map/pc.cpp` - function `pc_takeitem()`

```cpp
// หลังจากเก็บไอเทมสำเร็จ
bool from_mob = (fitem->first_get_charid != 0);
player_statistics_track_item_pickup(sd, &item_tmp, count, from_mob);
```

### 4.7 การฆ่ามอนสเตอร์

**ไฟล์**: `src/map/mob.cpp` - function `mob_damage()`

```cpp
// เมื่อมอนสเตอร์ตาย
if (md->hp <= 0 && src && src->type == BL_PC) {
    struct map_session_data* sd = BL_CAST(BL_PC, src);
    player_statistics_track_mob_kill(sd, md);
}
```

### 4.8 การใช้สกิล

**ไฟล์**: `src/map/skill.cpp` - function `skill_castend_id()` และ `skill_castend_pos2()`

```cpp
// หลังจาก cast skill สำเร็จ
int skill_type = 0; // 0=other, 1=offensive, 2=support
if (skill_get_inf(skill_id) & INF_ATTACK_SKILL) skill_type = 1;
else if (skill_get_inf(skill_id) & INF_SUPPORT_SKILL) skill_type = 2;
player_statistics_track_skill_use(sd, skill_id, skill_lv, skill_type);
```

---

## 5. ระบบ Auto-save

**ไฟล์**: `src/map/player_statistics.cpp`

```cpp
// Timer function ที่ทำงานทุก 5 นาที
void player_statistics_autosave_timer(int tid, int64 tick, int id, intptr_t data) {
    // วน loop ผู้เล่นทั้งหมดที่ออนไลน์
    map_foreachpc([](struct map_session_data* sd) {
        if (sd->statistics && sd->statistics->dirty) {
            player_statistics_save(sd->statistics, false);
            sd->statistics->dirty = false;
        }
        return 0;
    });

    // Re-register timer
    add_timer_interval(gettick() + 300000, player_statistics_autosave_timer, 0, 0, 300000);
}
```

---

## 6. Script Commands (สำหรับ NPC)

เพิ่ม script commands ใน `src/map/script.cpp`:

```cpp
// ดึงสถิติผู้เล่น
BUILDIN_FUNC(getplayerstat) {
    const char* stat_name = script_getstr(st, 2);
    struct map_session_data* sd = script_rid2sd(st);

    if (!sd || !sd->statistics)
        return SCRIPT_CMD_SUCCESS;

    uint64 value = player_statistics_get_stat(sd->status.char_id, stat_name);
    script_pushint(st, value);
    return SCRIPT_CMD_SUCCESS;
}

// ตัวอย่าง: .@kills = getplayerstat("mob_kill_count");
```

---

## 7. ตัวอย่างการใช้งาน (Usage Examples)

### SQL Queries สำหรับ Ranking

```sql
-- Top 10 Players by Online Time
SELECT char_id, online_time
FROM player_statistics
ORDER BY online_time DESC
LIMIT 10;

-- Top 10 Monster Hunters
SELECT c.name, ps.mob_kill_count, ps.mvp_kill_count
FROM player_statistics ps
JOIN char c ON c.char_id = ps.char_id
ORDER BY ps.mob_kill_count DESC
LIMIT 10;

-- Most Active Players Today
SELECT c.name, psd.online_time, psd.mob_kill, psd.exp_gained
FROM player_statistics_daily psd
JOIN char c ON c.char_id = psd.char_id
WHERE psd.stat_date = CURDATE()
ORDER BY psd.online_time DESC
LIMIT 10;
```

### NPC Script Example

```c
// Ranking NPC
prontera,150,150,5	script	Statistics Board	4_BOARD1,{
    mes "[Player Statistics]";
    mes "Your Statistics:";
    mes "-------------------";
    mes "Online Time: " + (getplayerstat("online_time") / 3600) + " hours";
    mes "Monsters Killed: " + getplayerstat("mob_kill_count");
    mes "Skills Used: " + getplayerstat("skill_used_count");
    mes "Items Picked: " + getplayerstat("item_picked_count");
    mes "Teleports: " + getplayerstat("teleport_count");
    close;
}
```

---

## 8. ข้อควรระวัง (Considerations)

### Performance
- ใช้ `dirty flag` เพื่อ save เฉพาะข้อมูลที่เปลี่ยน
- Auto-save ทุก 5 นาที (ปรับได้)
- Force save เมื่อ logout
- ใช้ index ใน database เพื่อเพิ่มความเร็ว query

### Storage
- สถิติหลักเก็บแบบ cumulative (ไม่มีวันหมดอายุ)
- สถิติรายวันเก็บไว้ 90 วัน (สามารถลบข้อมูลเก่าด้วย cron job)
- ข้อมูลละเอียด (detail table) เก็บเฉพาะข้อมูลที่จำเป็น

### Accuracy
- เวลาออนไลน์: อัปเดตทุก 1 นาที และเมื่อ logout
- การเคลื่อนไหว: นับทุกครั้งที่เริ่มเดิน (ไม่ใช่ทุก cell)
- Skill usage: นับเมื่อ cast สำเร็จ (ไม่ใช่ตอนเริ่ม cast)

---

## 9. แผนการพัฒนา (Implementation Plan)

### Phase 1: Core Structure (1-2 วัน)
- [ ] สร้างไฟล์ SQL (main.sql update)
- [ ] สร้าง player_statistics.hpp
- [ ] สร้าง player_statistics.cpp (load/save functions)
- [ ] เพิ่ม configuration ใน char.hpp และ char_athena.conf

### Phase 2: Integration (2-3 วัน)
- [ ] เชื่อมต่อ login/logout tracking
- [ ] เชื่อมต่อ item usage tracking
- [ ] เชื่อมต่อ movement tracking
- [ ] เชื่อมต่อ monster kill tracking
- [ ] เชื่อมต่อ skill usage tracking

### Phase 3: Advanced Features (1-2 วัน)
- [ ] ระบบ auto-save
- [ ] Daily statistics aggregation
- [ ] Detailed statistics (JSON data)
- [ ] Script commands

### Phase 4: Testing & Optimization (1-2 วัน)
- [ ] ทดสอบ performance
- [ ] ทดสอบ data accuracy
- [ ] เพิ่ม index ที่จำเป็น
- [ ] Documentation

---

## 10. ไฟล์ที่ต้องสร้าง/แก้ไข

### ไฟล์ใหม่:
- `sql-files/upgrades/player_statistics.sql` - SQL schema
- `src/map/player_statistics.hpp` - Header file
- `src/map/player_statistics.cpp` - Implementation
- `doc/player_statistics.md` - Documentation

### ไฟล์ที่ต้องแก้ไข:
- `sql-files/main.sql` - เพิ่ม tables
- `src/common/mmo.hpp` - เพิ่ม pointer ใน map_session_data
- `src/char/char.hpp` - เพิ่ม schema config
- `conf/char_athena.conf` - เพิ่ม configuration
- `src/map/pc.cpp` - Login/logout/item tracking
- `src/map/mob.cpp` - Monster kill tracking
- `src/map/skill.cpp` - Skill usage tracking
- `src/map/unit.cpp` - Movement tracking
- `src/map/battle.cpp` - Damage/heal tracking
- `src/map/script.cpp` - Script commands
- `src/map/CMakeLists.txt` - เพิ่ม player_statistics.cpp

---

## สรุป

ระบบนี้ออกแบบให้:
1. **ครบถ้วน**: ครอบคลุมสถิติทั้ง 8 ประเภทที่ต้องการ
2. **มีประสิทธิภาพ**: ใช้ dirty flag และ auto-save
3. **ขยายได้**: สามารถเพิ่มสถิติใหม่ได้ง่าย
4. **ใช้งานง่าย**: มี script commands สำหรับ NPC
5. **ตรวจสอบได้**: มี daily statistics และ detail tracking

คุณต้องการให้ฉันเริ่มพัฒนาระบบนี้เลยหรือไม่?
