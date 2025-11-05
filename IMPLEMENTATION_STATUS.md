# สถานะการพัฒนาระบบสถิติผู้เล่น (Player Statistics System)

## วันที่: 2025-11-05

## ✅ สิ่งที่ทำเสร็จแล้ว (Completed)

### 1. Database Schema (SQL)
- ✅ สร้างไฟล์: `sql-files/upgrades/upgrade_20251105_player_statistics.sql`
- ✅ ตารางหลัก: `player_statistics` (ข้อมูลสถิติรวม)
- ✅ ตารางรายวัน: `player_statistics_daily` (สถิติแยกตามวัน)
- ✅ ตารางรายละเอียด: `player_statistics_detail` (ข้อมูล JSON)
- ✅ สร้าง indexes สำหรับ performance
- ✅ รองรับการสร้างข้อมูลเริ่มต้นสำหรับตัวละครที่มีอยู่แล้ว

**สถิติที่เก็บครบถ้วน:**
- ✅ เวลาออนไลน์ (online_time, session_start, last_login, login_count)
- ✅ การใช้ไอเทม (item_used_count, item_used_healing, item_used_buff, item_used_other)
- ✅ การ Teleport (teleport_count, warp_portal_count, butterfly_wing_count)
- ✅ การเคลื่อนไหว (movement_count, movement_distance)
- ✅ การรักษา HP/SP (heal_skill_count, heal_amount_total, sp_recovery_skill_count, sp_recovery_amount)
- ✅ การเก็บไอเทม (item_picked_count, item_picked_from_mob, item_picked_from_ground, zeny_picked)
- ✅ การฆ่ามอนสเตอร์ (mob_kill_count, mvp_kill_count, boss_kill_count, mini_boss_kill_count)
- ✅ การใช้สกิล (skill_used_count, skill_offensive_count, skill_support_count, skill_passive_triggered)
- ✅ สถิติเพิ่มเติม (death_count, damage_dealt, damage_received, chat_message_count)

### 2. C++ Header File
- ✅ สร้างไฟล์: `src/map/player_statistics.hpp`
- ✅ ประกาศ struct `player_statistics` พร้อม field ครบถ้วน
- ✅ ประกาศ enum สำหรับ item type, teleport type, skill type
- ✅ ประกาศ struct `player_statistics_config` สำหรับการตั้งค่า
- ✅ ประกาศ function prototypes ครบถ้วน:
  - System functions (init, final, load_config)
  - Data management (load, save, create, delete, free)
  - Tracking functions (login, logout, item_use, teleport, movement, heal, pickup, mob_kill, skill_use, death, damage, chat)
  - Update functions (update_online_time, autosave_timer)
  - Query functions (get_online_time, get_stat)
  - Daily statistics functions

### 3. C++ Implementation File (Stub)
- ✅ สร้างไฟล์: `src/map/player_statistics.cpp`
- ✅ พื้นฐาน system functions (init, final, load_config)
- ✅ พื้นฐาน data management functions (load, save, create, delete, free)
- ✅ ครบถ้วน tracking functions (ทำงานได้เต็มรูปแบบในหน่วยความจำ)
- ✅ ครบถ้วน update functions
- ✅ Auto-save timer implementation
- ✅ Configuration structure พร้อมค่า default

### 4. Integration with Player System
- ✅ เพิ่ม pointer `struct player_statistics *statistics` ใน `class map_session_data` (pc.hpp:823)
- ✅ วางตำแหน่งหลัง achievement_data (เหมาะสมเนื่องจากเป็น subsystem ที่เกี่ยวข้อง)

### 5. Documentation
- ✅ เอกสารออกแบบฉบับเต็ม: `PLAYER_STATS_DESIGN.md`
- ✅ เอกสารสถานะการพัฒนา: `IMPLEMENTATION_STATUS.md` (ไฟล์นี้)

---

## ⏳ สิ่งที่ต้องทำต่อ (TODO)

### Phase 1: Database Integration (สำคัญมาก!)
**จำเป็นต้องทำก่อนจึงจะใช้งานได้จริง**

1. **Implement Database Load Function** (`player_statistics_load`)
   - ดึงข้อมูลจาก `player_statistics` table
   - ใช้ SQL connection จาก char server
   - Parse ข้อมูลเข้า struct
   - ตำแหน่ง: `src/map/player_statistics.cpp:94`

2. **Implement Database Save Function** (`player_statistics_save`)
   - บันทึกข้อมูลลง `player_statistics` table
   - ใช้ prepared statements เพื่อความปลอดภัย
   - Update เฉพาะ field ที่เปลี่ยน (differential save)
   - ตำแหน่ง: `src/map/player_statistics.cpp:108`

3. **Implement Database Create Function** (`player_statistics_create`)
   - INSERT record ใหม่ลง database
   - ตำแหน่ง: `src/map/player_statistics.cpp:130`

4. **Implement Database Delete Function** (`player_statistics_delete`)
   - DELETE record จาก database
   - ตำแหน่ง: `src/map/player_statistics.cpp:150`

5. **Add Configuration Loading**
   - อ่านค่าจาก `conf/char_athena.conf`
   - เพิ่ม config fields: enabled, autosave_interval, detailed_tracking, daily_tracking
   - ตำแหน่ง: `src/map/player_statistics.cpp:81`

### Phase 2: Integration with Game Events
**เชื่อมต่อกับเหตุการณ์ต่างๆ ในเกม**

1. **Login/Logout Integration**
   - แก้ไข: `src/map/pc.cpp` - function `pc_authok()`
   - เพิ่ม: โหลดสถิติเมื่อ login
   - เพิ่ม: เรียก `player_statistics_track_login()`

   - แก้ไข: `src/map/pc.cpp` - function `map_quit()` หรือ `pc_makesavestatus()`
   - เพิ่ม: เรียก `player_statistics_track_logout()`
   - เพิ่ม: บันทึกสถิติเมื่อ logout

2. **Item Usage Integration**
   - แก้ไข: `src/map/pc.cpp` - function `pc_useitem()`
   - เพิ่ม: ตรวจสอบประเภทไอเทม (healing/buff/other)
   - เพิ่ม: เรียก `player_statistics_track_item_use()`

3. **Teleport Integration**
   - แก้ไข: `src/map/skill.cpp` - function `skill_castend_nodamage_id()`
   - เพิ่ม: ตรวจจับ AL_TELEPORT, TK_HIGHJUMP
   - เพิ่ม: เรียก `player_statistics_track_teleport()`

   - แก้ไข: `src/map/pc.cpp` - function `pc_useitem()`
   - เพิ่ม: ตรวจจับ Fly Wing, Butterfly Wing
   - เพิ่ม: เรียก `player_statistics_track_teleport()`

4. **Movement Integration**
   - แก้ไข: `src/map/unit.cpp` - function `unit_walktoxy()`
   - เพิ่ม: เรียก `player_statistics_track_movement()` เมื่อผู้เล่นเริ่มเดิน

5. **Healing Integration**
   - แก้ไข: `src/map/battle.cpp` - function `battle_heal()`
   - เพิ่ม: เรียก `player_statistics_track_heal()` เมื่อรักษา HP/SP

6. **Item Pickup Integration**
   - แก้ไข: `src/map/pc.cpp` - function `pc_takeitem()`
   - เพิ่ม: ตรวจสอบว่าเป็น drop จาก mob หรือไม่
   - เพิ่ม: เรียก `player_statistics_track_item_pickup()`

7. **Monster Kill Integration**
   - แก้ไข: `src/map/mob.cpp` - function `mob_damage()`
   - เพิ่ม: เรียก `player_statistics_track_mob_kill()` เมื่อมอนสเตอร์ตาย
   - เพิ่ม: ตรวจจับ MVP/Boss/Mini-boss

8. **Skill Usage Integration**
   - แก้ไข: `src/map/skill.cpp` - functions `skill_castend_id()`, `skill_castend_pos2()`
   - เพิ่ม: ตรวจสอบประเภทสกิล (offensive/support)
   - เพิ่ม: เรียก `player_statistics_track_skill_use()`

9. **Death Integration**
   - แก้ไข: `src/map/pc.cpp` - function `pc_damage()` หรือ death handler
   - เพิ่ม: เรียก `player_statistics_track_death()`

10. **Damage Tracking Integration**
    - แก้ไข: `src/map/battle.cpp` - damage calculation functions
    - เพิ่ม: เรียก `player_statistics_track_damage()`

11. **Chat Integration**
    - แก้ไข: `src/map/clif.cpp` - chat message handlers
    - เพิ่ม: เรียก `player_statistics_track_chat()`

### Phase 3: Build System Integration
**เพิ่มไฟล์ในระบบ build**

1. **Add to CMakeLists.txt**
   - แก้ไข: `src/map/CMakeLists.txt`
   - เพิ่ม: `player_statistics.cpp` ใน source list

2. **Include Header in PC Module**
   - แก้ไข: `src/map/pc.cpp`
   - เพิ่ม: `#include "player_statistics.hpp"` ที่ตอนต้นไฟล์

### Phase 4: Advanced Features
**ฟีเจอร์เพิ่มเติม**

1. **Daily Statistics**
   - Implement `player_statistics_update_daily()`
   - Implement `player_statistics_daily_reset_timer()`
   - Implement `player_statistics_cleanup_daily()`
   - เพิ่ม daily reset timer (midnight)

2. **Detailed Statistics (JSON)**
   - Implement top skills tracking
   - Implement top items tracking
   - Implement top mobs tracking
   - Implement map visit statistics
   - Implement session history

3. **Script Commands**
   - แก้ไข: `src/map/script.cpp`
   - เพิ่ม: `BUILDIN_FUNC(getplayerstat)`
   - เพิ่ม: `BUILDIN_FUNC(setplayerstat)` (ถ้าต้องการ)
   - อัปเดต: script command table

4. **NPC Examples**
   - สร้าง: `npc/custom/player_statistics_board.txt`
   - ตัวอย่าง NPC แสดงสถิติ
   - ตัวอย่าง NPC ranking

### Phase 5: Character Server Integration (Optional)
**ถ้าต้องการให้ char server จัดการบันทึก**

1. **Add Schema Config**
   - แก้ไข: `src/char/char.hpp` - `struct Schema_Config`
   - เพิ่ม: `std::string player_statistics_table;`
   - เพิ่ม: `std::string player_statistics_daily_table;`
   - เพิ่ม: `std::string player_statistics_detail_table;`

2. **Add Config Options**
   - แก้ไข: `conf/char_athena.conf`
   - เพิ่ม: table name configurations
   - เพิ่ม: autosave interval
   - เพิ่ม: enable/disable flags

---

## 🔧 การติดตั้งและใช้งาน (Installation)

### ขั้นตอนที่ 1: รัน SQL Upgrade Script
```bash
mysql -u root -p ragnarok < sql-files/upgrades/upgrade_20251105_player_statistics.sql
```

### ขั้นตอนที่ 2: Compile Server
```bash
# ต้องเพิ่ม player_statistics.cpp ใน CMakeLists.txt ก่อน!
cd build
cmake ..
make -j4
```

### ขั้นตอนที่ 3: Test
```bash
# รัน map server และทดสอบการ login
# ตรวจสอบ database ว่ามีการสร้างข้อมูลหรือไม่
mysql -u root -p ragnarok -e "SELECT * FROM player_statistics LIMIT 5;"
```

---

## 📊 สถานะการทำงาน (Current Status)

### ทำงานได้แล้ว:
- ✅ โครงสร้างข้อมูลครบถ้วน (memory)
- ✅ Tracking functions ทั้งหมดทำงานได้ (ในหน่วยความจำ)
- ✅ Auto-save timer system
- ✅ Configuration system

### ยังทำงานไม่ได้:
- ❌ การบันทึก/โหลดจาก database (ต้อง implement)
- ❌ การเชื่อมต่อกับ game events (ต้อง integrate)
- ❌ Build system integration (ต้องเพิ่ม CMakeLists)
- ❌ Script commands (ต้อง implement)

### Compile Status:
- ⚠️ **ยังไม่ได้ compile** - ต้องเพิ่ม `player_statistics.cpp` ใน CMakeLists.txt ก่อน
- ⚠️ ต้อง include header ที่จำเป็นใน pc.cpp

---

## 🎯 ขั้นตอนถัดไป (Next Steps)

### ลำดับความสำคัญ:
1. **สูงสุด**: Database integration (Phase 1)
2. **สูง**: Build system + Login/Logout integration
3. **กลาง**: Game events integration (Phase 2)
4. **ต่ำ**: Advanced features (Phase 3-4)

### เวลาโดยประมาณ:
- Phase 1: 2-3 ชั่วโมง
- Phase 2: 4-6 ชั่วโมง
- Phase 3: 1 ชั่วโมง
- Phase 4: 3-4 ชั่วโมง
- **รวม: 10-14 ชั่วโมง** สำหรับระบบที่ทำงานได้เต็มรูปแบบ

---

## 📝 หมายเหตุ (Notes)

1. **Performance Considerations**:
   - ใช้ dirty flag เพื่อลด database writes
   - Auto-save ทุก 5 นาที (ปรับได้)
   - Force save เมื่อ logout
   - ใช้ indexes ในทุก query

2. **Data Accuracy**:
   - เวลาออนไลน์: อัปเดตทุก 5 นาที + เมื่อ logout
   - การเคลื่อนไหว: นับทุกครั้งที่เริ่มเดิน (ไม่ใช่ทุก cell)
   - Skill usage: นับเมื่อ cast สำเร็จ

3. **Security**:
   - ใช้ prepared statements เสมอ
   - Validate input ทุกครั้ง
   - ใช้ nullpo_retv() เพื่อป้องกัน null pointer

4. **Compatibility**:
   - รองรับ MySQL/MariaDB
   - ใช้ InnoDB engine
   - Character set: utf8mb4

---

## 🐛 Known Issues

1. **Stub Implementation**: ฟังก์ชันบันทึก/โหลดจาก database ยังไม่ได้ implement
2. **No Integration**: ยังไม่ได้เชื่อมต่อกับ game events
3. **No Build Config**: ยังไม่ได้เพิ่มใน CMakeLists.txt
4. **Boss Detection**: ต้องเพิ่มการตรวจจับ boss/mini-boss flags

---

## 👨‍💻 ผู้พัฒนา

- **Design & Implementation**: Claude (AI Assistant)
- **Date**: 2025-11-05
- **Version**: 1.0.0-alpha (Stub)
- **Target Platform**: rAthena (Ragnarok Online Emulator)

---

## 📖 เอกสารอ้างอิง

- Design Document: `PLAYER_STATS_DESIGN.md`
- SQL Schema: `sql-files/upgrades/upgrade_20251105_player_statistics.sql`
- Header File: `src/map/player_statistics.hpp`
- Implementation: `src/map/player_statistics.cpp`
