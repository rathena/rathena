#ifdef HAVE_TS_SCRIPTING

#include "host_world.hpp"

#include "arg_helpers.hpp"
#include "../atcommand.hpp"
#include "../battle.hpp"
#include "../clif.hpp"
#include "../guild.hpp"
#include "../itemdb.hpp"
#include "../map.hpp"
#include "../mob.hpp"
#include "../npc.hpp"
#include "../pc.hpp"
#include "../status.hpp"
#include "../unit.hpp"
#include "../../common/mapindex.hpp"
#include "../../common/showmsg.hpp"
#include "../../common/timer.hpp"

namespace rathena::scripting {

namespace {
using namespace args;

// Most ops don't need an attached player; those that do can early-out
// when self->sd() is null.
#define UNWRAP_OPT auto* self = args::unwrap<WorldHost>(info); if (!self) return;
#define UNWRAP_REQ \
    UNWRAP_OPT; \
    auto* sd = self->sd(); \
    if (!sd) return;
} // namespace

// =====================================================================
// Time
// =====================================================================

void WorldHost::now_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    ret_int(info, static_cast<int32_t>(gettick()));
}

void WorldHost::getTime_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int type = int_arg(info, 0);
    time_t t = time(nullptr);
    struct tm* lt = localtime(&t);
    if (!lt) { ret_int(info, 0); return; }
    int v = 0;
    switch (type) {
        case 1: v = lt->tm_sec; break;
        case 2: v = lt->tm_min; break;
        case 3: v = lt->tm_hour; break;
        case 4: v = lt->tm_wday; break;
        case 5: v = lt->tm_mday; break;
        case 6: v = lt->tm_mon + 1; break;
        case 7: v = lt->tm_year + 1900; break;
        case 8: v = lt->tm_yday + 1; break;
        default: v = static_cast<int>(t); break;
    }
    ret_int(info, v);
}
void WorldHost::getTimeTick_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    ret_int(info, static_cast<int32_t>(time(nullptr)));
}
void WorldHost::getTimeStr_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto fmt = str_arg(info, 0, "%Y-%m-%d %H:%M:%S");
    int maxlen = int_arg(info, 1, 64);
    if (maxlen <= 0 || maxlen > 1024) maxlen = 64;
    std::string buf;
    buf.resize(static_cast<size_t>(maxlen));
    time_t t = time(nullptr);
    struct tm* lt = localtime(&t);
    if (lt) {
        size_t n = strftime(&buf[0], buf.size(), fmt.c_str(), lt);
        buf.resize(n);
    } else {
        buf.clear();
    }
    ret_str(info, buf);
}

// =====================================================================
// Announce family
// =====================================================================

void WorldHost::announce_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_OPT;
    auto msg = str_arg(info, 0);
    if (self->sd()) {
        clif_broadcast(&self->sd()->bl, msg.c_str(),
                       static_cast<int32>(msg.size() + 1), BC_DEFAULT, ALL_CLIENT);
    }
}

void WorldHost::mapAnnounce_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_OPT;
    (void)str_arg(info, 0);
    auto msg = str_arg(info, 1);
    if (self->sd()) {
        clif_broadcast(&self->sd()->bl, msg.c_str(),
                       static_cast<int32>(msg.size() + 1), BC_DEFAULT, ALL_SAMEMAP);
    }
}
void WorldHost::areaAnnounce_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_OPT;
    (void)str_arg(info, 0);
    auto msg = str_arg(info, 5);
    if (self->sd()) {
        clif_broadcast(&self->sd()->bl, msg.c_str(),
                       static_cast<int32>(msg.size() + 1), BC_DEFAULT, AREA);
    }
}

void WorldHost::globalMessage_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_REQ;
    auto msg = str_arg(info, 0);
    clif_GlobalMessage(sd->bl, msg.c_str(), AREA_CHAT_WOC);
}
void WorldHost::debugMessage_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto msg = str_arg(info, 0);
    ShowDebug("[ts-scripting] %s\n", msg.c_str());
}
void WorldHost::errorMessage_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto msg = str_arg(info, 0);
    ShowError("[ts-scripting] %s\n", msg.c_str());
}
void WorldHost::logMessage_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto msg = str_arg(info, 0);
    ShowInfo("[ts-scripting] %s\n", msg.c_str());
}

// =====================================================================
// Sound / BGM
// =====================================================================

void WorldHost::soundEffectAll_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { (void)info; }
void WorldHost::playBgmAll_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     { (void)info; }

// =====================================================================
// Spawning
// =====================================================================

void WorldHost::spawnMob_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_OPT;
    auto mname = str_arg(info, 0);
    int x  = int_arg(info, 1);
    int y  = int_arg(info, 2);
    auto display = str_arg(info, 3);
    int mob = int_arg(info, 4);
    int amount = int_arg(info, 5, 1);
    auto event = str_arg(info, 6);
    int gid = mob_once_spawn(self->sd(), map_mapname2mapid(mname.c_str()), x, y,
                             display.c_str(), mob, amount, event.c_str(),
                             SZ_SMALL, AI_NONE);
    ret_int(info, gid);
}
void WorldHost::spawnAreaMob_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_OPT;
    auto mname = str_arg(info, 0);
    int x1 = int_arg(info, 1), y1 = int_arg(info, 2);
    int x2 = int_arg(info, 3), y2 = int_arg(info, 4);
    auto display = str_arg(info, 5);
    int mob = int_arg(info, 6);
    int amount = int_arg(info, 7, 1);
    auto event = str_arg(info, 8);
    int gid = mob_once_spawn_area(self->sd(), map_mapname2mapid(mname.c_str()),
                                  x1, y1, x2, y2, display.c_str(), mob, amount,
                                  event.c_str(), SZ_SMALL, AI_NONE);
    ret_int(info, gid);
}
void WorldHost::spawnGuardian_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    ret_int(info, 0);  // TODO: mob_spawn_guardian
}
void WorldHost::guardianInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    ret_null(info);
}

void WorldHost::killMonster_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    ret_int(info, 0);  // TODO: map_foreachinmap + atcommand_killmonster_sub
}
void WorldHost::killMonsterAll_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    (void)str_arg(info, 0);
    // TODO: map_foreachinmap with a kill-mob lambda — atcommand_killmonster_sub
    // is file-static in atcommand.cpp; the simpler path is a local lambda.
    ret_int(info, 0);
}
void WorldHost::mobCount_cb(const v8::FunctionCallbackInfo<v8::Value>& info)        { ret_int(info, 0); }
void WorldHost::respawnGuildOwned_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ (void)info; }
void WorldHost::getRandomMobId_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { ret_int(info, 0); }
void WorldHost::getMonsterInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { ret_null(info); }
void WorldHost::getMobDrops_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     {
    info.GetReturnValue().Set(v8::Array::New(iso(info), 0));
}
void WorldHost::mobInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info)         { ret_str(info, ""); }

// =====================================================================
// Unit-level ops — all placeholders for now (need unit_walktoxy / etc.)
// =====================================================================

void WorldHost::unitWalk_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    int x = int_arg(info, 1), y = int_arg(info, 2);
    auto* bl = map_id2bl(gid);
    if (bl) unit_walktoxy(bl, x, y, 0);
}
void WorldHost::unitWalkToTarget_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    int tid = int_arg(info, 1);
    auto* src = map_id2bl(gid);
    auto* tgt = map_id2bl(tid);
    if (src && tgt) unit_walktobl(src, tgt, 1, 0);
}
void WorldHost::unitAttack_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    int tid = int_arg(info, 1);
    int cont = int_arg(info, 2, 1);
    auto* src = map_id2bl(gid);
    if (src) unit_attack(src, tid, cont);
}
void WorldHost::unitKill_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    auto* bl = map_id2bl(gid);
    if (bl) status_kill(bl);
}
void WorldHost::unitWarp_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    auto mname = str_arg(info, 1);
    int x = int_arg(info, 2), y = int_arg(info, 3);
    auto* bl = map_id2bl(gid);
    if (bl) unit_warp(bl, map_mapname2mapid(mname.c_str()), x, y, CLR_TELEPORT);
}
void WorldHost::unitTalk_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    auto text = str_arg(info, 1);
    auto* bl = map_id2bl(gid);
    if (bl) clif_disp_overhead(bl, text.c_str());
}
void WorldHost::unitSkillUseId_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    int sid = int_arg(info, 1);
    int sl  = int_arg(info, 2, 1);
    auto* src = map_id2bl(gid);
    if (src) unit_skilluse_id(src, gid, sid, sl);
}
void WorldHost::unitSkillUsePos_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    int sid = int_arg(info, 1);
    int sl  = int_arg(info, 2, 1);
    int x   = int_arg(info, 3), y = int_arg(info, 4);
    auto* src = map_id2bl(gid);
    if (src) unit_skilluse_pos(src, x, y, sid, sl);
}
void WorldHost::unitStopAttack_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    auto* bl = map_id2bl(gid);
    if (bl) unit_stop_attack(bl);
}
void WorldHost::unitStopWalk_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    auto* bl = map_id2bl(gid);
    if (bl) unit_stop_walking(bl, 1);
}
void WorldHost::unitExists_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    ret_bool(info, map_id2bl(gid) != nullptr);
}
void WorldHost::getUnitType_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    auto* bl = map_id2bl(gid);
    ret_int(info, bl ? static_cast<int>(bl->type) : 0);
}
void WorldHost::getUnitName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    auto* bl = map_id2bl(gid);
    ret_str(info, bl ? status_get_name(*bl) : "");
}
void WorldHost::setUnitName_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { (void)info; }
void WorldHost::getUnitTitle_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { ret_str(info, ""); }
void WorldHost::setUnitTitle_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { (void)info; }
void WorldHost::getUnitData_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { ret_null(info); }
void WorldHost::setUnitData_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { (void)info; }
void WorldHost::getUnits_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     {
    info.GetReturnValue().Set(v8::Array::New(iso(info), 0));
}
void WorldHost::getMapUnits_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    info.GetReturnValue().Set(v8::Array::New(iso(info), 0));
}
void WorldHost::getAreaUnits_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().Set(v8::Array::New(iso(info), 0));
}

// =====================================================================
// User / area queries
// =====================================================================

void WorldHost::getMapUsers_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto mname = str_arg(info, 0);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m < 0) { ret_int(info, 0); return; }
    auto* md = map_getmapdata(m);
    ret_int(info, md ? md->users : 0);
}
void WorldHost::getAreaUsers_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto mname = str_arg(info, 0);
    int x1 = int_arg(info, 1), y1 = int_arg(info, 2);
    int x2 = int_arg(info, 3), y2 = int_arg(info, 4);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m < 0) { ret_int(info, 0); return; }
    ret_int(info, map_foreachinarea(nullptr, m, x1, y1, x2, y2, BL_PC));
}
void WorldHost::getServerUsers_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    ret_int(info, map_getusers());
}
void WorldHost::isLoggedIn_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int aid = int_arg(info, 0);
    ret_bool(info, map_id2sd(aid) != nullptr);
}
void WorldHost::ridToName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int rid = int_arg(info, 0);
    auto* sd = map_id2sd(rid);
    ret_str(info, sd ? sd->status.name : "");
}
void WorldHost::getAreaDropItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().Set(v8::Array::New(iso(info), 0));
}

// =====================================================================
// Map / location
// =====================================================================

void WorldHost::mapIdToName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int id = int_arg(info, 0);
    auto* n = map_mapid2mapname(id);
    ret_str(info, n ? n : "");
}
void WorldHost::getMapXY_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    auto* bl = map_id2bl(gid);
    if (!bl) { ret_null(info); return; }
    auto iso_ = iso(info); auto cx = ctx(info);
    auto obj = v8::Object::New(iso_);
    auto* mn = map_mapid2mapname(bl->m);
    (void)obj->Set(cx, v8::String::NewFromUtf8(iso_, "map").ToLocalChecked(),
                   v8::String::NewFromUtf8(iso_, mn ? mn : "").ToLocalChecked());
    (void)obj->Set(cx, v8::String::NewFromUtf8(iso_, "x").ToLocalChecked(),
                   v8::Integer::New(iso_, bl->x));
    (void)obj->Set(cx, v8::String::NewFromUtf8(iso_, "y").ToLocalChecked(),
                   v8::Integer::New(iso_, bl->y));
    info.GetReturnValue().Set(obj);
}
void WorldHost::distance_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int x0 = int_arg(info, 0), y0 = int_arg(info, 1);
    int x1 = int_arg(info, 2), y1 = int_arg(info, 3);
    int dx = x1 - x0, dy = y1 - y0;
    ret_int(info, (dx*dx + dy*dy));
}
void WorldHost::setCell_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    { (void)info; }
void WorldHost::checkCell_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { ret_int(info, 0); }
void WorldHost::getFreeCell_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ ret_null(info); }
void WorldHost::setWall_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    { (void)info; }
void WorldHost::delWall_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    { (void)info; }
void WorldHost::checkWall_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { ret_bool(info, false); }
void WorldHost::makeItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    t_itemid id = static_cast<t_itemid>(uint_arg(info, 0));
    int amount  = int_arg(info, 1, 1);
    auto mname  = str_arg(info, 2);
    int x = int_arg(info, 3), y = int_arg(info, 4);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m < 0 || amount <= 0) return;
    struct item it = {};
    it.nameid = id;
    it.amount = static_cast<int16>(amount);
    it.identify = 1;
    map_addflooritem(&it, amount, m, x, y, 0, 0, 0, 0, 0);
}
void WorldHost::cleanArea_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { (void)info; }
void WorldHost::cleanMap_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }
void WorldHost::warpPortal_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { (void)info; }
void WorldHost::mapWarp_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    { (void)info; }
void WorldHost::areaWarp_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }
void WorldHost::warpParty_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { (void)info; }
void WorldHost::warpGuild_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { (void)info; }
void WorldHost::areaPercentHeal_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ (void)info; }
void WorldHost::attachRid_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { (void)info; }
void WorldHost::addRid_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     { (void)info; }
void WorldHost::playerAttached_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_OPT;
    ret_int(info, self->sd() ? self->sd()->bl.id : 0);
}
void WorldHost::getAttachedRid_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    playerAttached_cb(info);
}

// =====================================================================
// Mapflags / day / pvp / gvg / agit
// =====================================================================

void WorldHost::setMapFlag_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto mname = str_arg(info, 0);
    int flag = int_arg(info, 1);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m >= 0) map_setmapflag(m, static_cast<e_mapflag>(flag), true);
}
void WorldHost::removeMapFlag_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto mname = str_arg(info, 0);
    int flag = int_arg(info, 1);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m >= 0) map_setmapflag(m, static_cast<e_mapflag>(flag), false);
}
void WorldHost::getMapFlag_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto mname = str_arg(info, 0);
    int flag = int_arg(info, 1);
    int16 m = map_mapname2mapid(mname.c_str());
    ret_int(info, m >= 0 ? map_getmapflag(m, static_cast<e_mapflag>(flag)) : 0);
}
void WorldHost::setMapFlagNoSave_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ (void)info; }

void WorldHost::day_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; map_night_timer(0, 0, 0, 1); }
void WorldHost::night_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { (void)info; map_day_timer(0, 0, 0, 1); }
void WorldHost::isDay_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { ret_bool(info, !night_flag); }
void WorldHost::isNight_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { ret_bool(info, night_flag); }

static inline void toggle_mapflag(const v8::FunctionCallbackInfo<v8::Value>& info,
                                   e_mapflag flag, bool on) {
    auto mname = args::str_arg(info, 0);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m < 0) return;
    if (map_getmapflag(m, flag) != (on ? 1 : 0)) map_setmapflag(m, flag, on);
}
void WorldHost::pvpOn_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { toggle_mapflag(info, MF_PVP, true); }
void WorldHost::pvpOff_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { toggle_mapflag(info, MF_PVP, false); }
void WorldHost::gvgOn_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { toggle_mapflag(info, MF_GVG, true); }
void WorldHost::gvgOff_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { toggle_mapflag(info, MF_GVG, false); }
void WorldHost::gvgOn3_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { toggle_mapflag(info, MF_GVG_TE, true); }
void WorldHost::gvgOff3_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { toggle_mapflag(info, MF_GVG_TE, false); }
void WorldHost::agitStart_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int era = int_arg(info, 0, 1);
    if (era >= 3) guild_agit3_start(); else if (era == 2) guild_agit2_start(); else guild_agit_start();
}
void WorldHost::agitEnd_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int era = int_arg(info, 0, 1);
    if (era >= 3) guild_agit3_end(); else if (era == 2) guild_agit2_end(); else guild_agit_end();
}
void WorldHost::agitCheck_cb(const v8::FunctionCallbackInfo<v8::Value>& info){
    int era = int_arg(info, 0, 1);
    bool on = (era >= 3) ? agit3_flag : (era == 2 ? agit2_flag : agit_flag);
    ret_bool(info, on);
}
void WorldHost::flagEmblem_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ (void)info; }
void WorldHost::castleName_cb(const v8::FunctionCallbackInfo<v8::Value>& info){
    auto mname = str_arg(info, 0);
    auto castle = castle_db.mapname2gc(mname.c_str());
    ret_str(info, castle ? castle->castle_name : "");
}
void WorldHost::castleData_cb(const v8::FunctionCallbackInfo<v8::Value>& info){
    auto mname = str_arg(info, 0);
    int type = int_arg(info, 1);
    auto castle = castle_db.mapname2gc(mname.c_str());
    if (!castle) { ret_int(info, 0); return; }
    int v = 0;
    switch (type) {
        case 1: v = castle->guild_id; break;
        case 2: v = castle->economy; break;
        case 3: v = castle->defense; break;
        case 4: v = castle->triggerE; break;
        case 5: v = castle->triggerD; break;
        case 6: v = castle->nextTime; break;
        case 7: v = castle->payTime; break;
        case 8: v = castle->createTime; break;
        case 9: v = castle->visibleC; break;
        default:
            if (type >= 10 && type <= 17) v = castle->guardian[type - 10].visible;
            break;
    }
    ret_int(info, v);
}

// =====================================================================
// Battle / at-commands
// =====================================================================

void WorldHost::setBattleFlag_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name = str_arg(info, 0);
    int val   = int_arg(info, 1);
    battle_set_value(name.c_str(), std::to_string(val).c_str());
}
void WorldHost::getBattleFlag_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name = str_arg(info, 0);
    ret_int(info, battle_get_value(name.c_str()));
}

void WorldHost::atCommand_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_REQ;
    auto cmd = str_arg(info, 0);
    is_atcommand(sd->fd, sd, cmd.c_str(), 1);
}
void WorldHost::charCommand_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { atCommand_cb(info); }
void WorldHost::useAtCommand_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ atCommand_cb(info); }
void WorldHost::bindAtCommand_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { (void)info; }
void WorldHost::unbindAtCommand_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ (void)info; }

// =====================================================================
// Game info
// =====================================================================

void WorldHost::itemName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    t_itemid id = static_cast<t_itemid>(uint_arg(info, 0));
    auto data = item_db.find(id);
    ret_str(info, data ? data->ename.c_str() : "");
}
void WorldHost::itemSlots_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    t_itemid id = static_cast<t_itemid>(uint_arg(info, 0));
    auto data = item_db.find(id);
    ret_int(info, data ? data->slots : 0);
}
void WorldHost::itemInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    t_itemid id = static_cast<t_itemid>(uint_arg(info, 0));
    int type = int_arg(info, 1);
    auto data = item_db.find(id);
    if (!data) { ret_int(info, 0); return; }
    // Subset of ITEMINFO_* type codes. See script.cpp:buildin_getiteminfo.
    switch (type) {
        case 0: ret_int(info, data->value_buy); return;
        case 1: ret_int(info, data->value_sell); return;
        case 2: ret_int(info, static_cast<int>(data->type)); return;
        case 3: ret_int(info, static_cast<int>(data->maxchance)); return;
        case 4: ret_int(info, static_cast<int>(data->sex)); return;
        case 5: ret_int(info, static_cast<int>(data->equip)); return;
        case 6: ret_int(info, data->weight); return;
        case 7: ret_int(info, data->atk); return;
        case 8: ret_int(info, data->def); return;
        case 9: ret_int(info, data->range); return;
        case 10: ret_int(info, data->slots); return;
        case 11: ret_int(info, static_cast<int>(data->look)); return;
        case 12: ret_int(info, data->elv); return;
        case 13: ret_int(info, static_cast<int>(data->weapon_level)); return;
        case 14: ret_int(info, data->view_id); return;
        default: ret_int(info, 0); return;
    }
}
void WorldHost::setItemInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }
void WorldHost::setItemScript_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { (void)info; }
void WorldHost::gmLevel_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_OPT;
    ret_int(info, self->sd() ? self->sd()->group_id : 0);
}
void WorldHost::groupId_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { gmLevel_cb(info); }
void WorldHost::itemLink_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { ret_str(info, ""); }

// =====================================================================
// install_on_object
// =====================================================================

void WorldHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                  v8::Local<v8::Object> obj) {
#define BIND(name) args::bind_method<WorldHost>(iso, ctx, obj, this, #name, &WorldHost::name##_cb)
    BIND(now);
    BIND(announce); BIND(mapAnnounce); BIND(areaAnnounce);
    BIND(globalMessage); BIND(debugMessage); BIND(errorMessage); BIND(logMessage);
    BIND(soundEffectAll); BIND(playBgmAll);
    BIND(spawnMob); BIND(spawnAreaMob); BIND(spawnGuardian); BIND(guardianInfo);
    BIND(killMonster); BIND(killMonsterAll); BIND(mobCount);
    BIND(respawnGuildOwned); BIND(getRandomMobId);
    BIND(getMonsterInfo); BIND(getMobDrops); BIND(mobInfo);
    BIND(unitWalk); BIND(unitWalkToTarget); BIND(unitAttack);
    BIND(unitKill); BIND(unitWarp); BIND(unitTalk);
    BIND(unitSkillUseId); BIND(unitSkillUsePos);
    BIND(unitStopAttack); BIND(unitStopWalk);
    BIND(unitExists); BIND(getUnitType); BIND(getUnitName);
    BIND(setUnitName); BIND(getUnitTitle); BIND(setUnitTitle);
    BIND(getUnitData); BIND(setUnitData);
    BIND(getUnits); BIND(getMapUnits); BIND(getAreaUnits);
    BIND(getMapUsers); BIND(getAreaUsers); BIND(getServerUsers);
    BIND(isLoggedIn); BIND(ridToName); BIND(getAreaDropItem);
    BIND(mapIdToName); BIND(getMapXY); BIND(distance);
    BIND(setCell); BIND(checkCell); BIND(getFreeCell);
    BIND(setWall); BIND(delWall); BIND(checkWall);
    BIND(makeItem); BIND(cleanArea); BIND(cleanMap);
    BIND(warpPortal); BIND(mapWarp); BIND(areaWarp);
    BIND(warpParty); BIND(warpGuild); BIND(areaPercentHeal);
    BIND(attachRid); BIND(addRid); BIND(playerAttached); BIND(getAttachedRid);
    BIND(setMapFlag); BIND(removeMapFlag); BIND(getMapFlag); BIND(setMapFlagNoSave);
    BIND(day); BIND(night); BIND(isDay); BIND(isNight);
    BIND(pvpOn); BIND(pvpOff); BIND(gvgOn); BIND(gvgOff); BIND(gvgOn3); BIND(gvgOff3);
    BIND(agitStart); BIND(agitEnd); BIND(agitCheck);
    BIND(flagEmblem); BIND(castleName); BIND(castleData);
    BIND(getTime); BIND(getTimeTick); BIND(getTimeStr);
    BIND(setBattleFlag); BIND(getBattleFlag);
    BIND(atCommand); BIND(charCommand); BIND(useAtCommand);
    BIND(bindAtCommand); BIND(unbindAtCommand);
    BIND(itemName); BIND(itemSlots); BIND(itemInfo);
    BIND(setItemInfo); BIND(setItemScript);
    BIND(gmLevel); BIND(groupId); BIND(itemLink);
#undef BIND
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
