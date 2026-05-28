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
#include "../homunculus.hpp"
#include "../party.hpp"
#include "../pc.hpp"
#include "../pet.hpp"
#include "../script.hpp"
#include "../skill.hpp"
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

namespace {
int32 soundeffect_map_sub(struct block_list* bl, va_list ap) {
    const char* name = va_arg(ap, const char*);
    int32 type       = va_arg(ap, int32);
    if (bl && bl->type == BL_PC) clif_soundeffect(*bl, name, type, SELF);
    return 0;
}
int32 playbgm_map_sub(map_session_data* sd, va_list ap) {
    const char* name = va_arg(ap, const char*);
    if (sd) clif_playBGM(*sd, name);
    return 0;
}
} // namespace

void WorldHost::soundEffectAll_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto fn = str_arg(info, 0);
    int type = int_arg(info, 1, 0);
    if (info.Length() >= 3 && info[2]->IsString()) {
        auto map_name = str_arg(info, 2);
        int m = map_mapname2mapid(map_name.c_str());
        if (m < 0) return;
        map_foreachinmap(soundeffect_map_sub, m, BL_PC, fn.c_str(), type);
    }
}
void WorldHost::playBgmAll_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     {
    auto fn = str_arg(info, 0);
    map_foreachpc(playbgm_map_sub, fn.c_str());
}

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

namespace {
int32 killmob_sub_strip(struct block_list* bl, va_list ap) {
    auto* md = BL_CAST(BL_MOB, bl);
    const char* event = va_arg(ap, const char*);
    int allflag = va_arg(ap, int);
    if (!md) return 0;
    md->state.npc_killmonster = 1;
    if (!allflag) {
        if (strcmp(event, md->npc_event) == 0) status_kill(bl);
    } else if (!md->spawn) {
        status_kill(bl);
    }
    md->state.npc_killmonster = 0;
    return 0;
}
int32 killmob_all_sub(struct block_list* bl, va_list ap) {
    (void)ap;
    auto* md = BL_CAST(BL_MOB, bl);
    if (md) { strcpy(md->npc_event, ""); status_kill(bl); }
    return 0;
}
int32 mobcount_sub(struct block_list* bl, va_list ap) {
    auto* md = BL_CAST(BL_MOB, bl);
    const char* event = va_arg(ap, const char*);
    if (!md || md->status.hp < 1) return 0;
    if (event && strcmp(event, md->npc_event) != 0) return 0;
    return 1;
}
} // namespace

void WorldHost::killMonster_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto mapn = str_arg(info, 0);
    auto event = str_arg(info, 1);
    int allflag = (event == "All") ? 1 : 0;
    int m = map_mapname2mapid(mapn.c_str());
    if (m < 0) return;
    map_freeblock_lock();
    map_foreachinmap(killmob_sub_strip, m, BL_MOB, event.c_str(), allflag);
    map_freeblock_unlock();
}
void WorldHost::killMonsterAll_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto mapn = str_arg(info, 0);
    int m = map_mapname2mapid(mapn.c_str());
    if (m < 0) return;
    map_foreachinmap(killmob_all_sub, m, BL_MOB);
}
void WorldHost::mobCount_cb(const v8::FunctionCallbackInfo<v8::Value>& info)        {
    auto mapn = str_arg(info, 0);
    auto event = str_arg(info, 1);
    int m = map_mapname2mapid(mapn.c_str());
    if (m < 0) { ret_int(info, 0); return; }
    const char* ev = (event == "all" || event.empty()) ? nullptr : event.c_str();
    ret_int(info, map_foreachinmap(mobcount_sub, m, BL_MOB, ev));
}
namespace {
int32 respawnguild_pc_sub(map_session_data* sd, va_list ap) {
    int16 m  = va_arg(ap, int32);
    int32 gid = va_arg(ap, int32);
    int32 flag = va_arg(ap, int32);
    if (!sd || sd->bl.m != m) return 0;
    if ((static_cast<int32>(sd->status.guild_id) == gid && flag & 1) ||
        (static_cast<int32>(sd->status.guild_id) != gid && flag & 2) ||
        (sd->status.guild_id == 0 && flag & 2)) {
        pc_setpos(sd, mapindex_name2id(sd->status.save_point.map),
                  sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
    }
    return 1;
}
int32 respawnguild_mob_sub(struct block_list* bl, va_list ap) {
    (void)ap;
    auto* md = reinterpret_cast<mob_data*>(bl);
    if (!md->guardian_data && md->mob_id != MOBID_EMPERIUM &&
        (!mob_is_clone(md->mob_id) || battle_config.guild_maprespawn_clones))
        status_kill(bl);
    return 1;
}
} // namespace

void WorldHost::respawnGuildOwned_cb(const v8::FunctionCallbackInfo<v8::Value>& info){
    auto mname = str_arg(info, 0);
    int gid    = int_arg(info, 1);
    int flag   = int_arg(info, 2, 3);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m < 0) return;
    map_foreachpc(respawnguild_pc_sub, m, gid, flag);
    if (flag & 4) map_foreachinmap(respawnguild_mob_sub, m, BL_MOB);
}
void WorldHost::getRandomMobId_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    int type = int_arg(info, 0);
    int flag = int_arg(info, 1, RMF_NONE);
    int lv   = int_arg(info, 2, 0);
    ret_int(info, mob_get_random_id(type, static_cast<e_random_monster_flags>(flag), lv));
}
void WorldHost::getMonsterInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    int id = int_arg(info, 0);
    int type = int_arg(info, 1);
    if (mob_is_clone(id)) { ret_null(info); return; }
    auto mob = mob_db.find(id);
    if (!mob) {
        if (type == MOB_NAME) ret_str(info, "null");
        else                  ret_int(info, -1);
        return;
    }
    switch (type) {
        case MOB_NAME:    ret_str(info, mob->jname.c_str()); return;
        case MOB_LV:      ret_int(info, mob->lv); return;
        case MOB_MAXHP:   ret_int(info, mob->status.max_hp); return;
        case MOB_BASEEXP: ret_int(info, mob->base_exp); return;
        case MOB_JOBEXP:  ret_int(info, mob->job_exp); return;
        case MOB_ATK1:    ret_int(info, mob->status.rhw.atk); return;
        case MOB_ATK2:    ret_int(info, mob->status.rhw.atk2); return;
        case MOB_DEF:     ret_int(info, mob->status.def); return;
        case MOB_MDEF:    ret_int(info, mob->status.mdef); return;
        case MOB_RACE:    ret_int(info, mob->status.race); return;
        case MOB_ELEMENT: ret_int(info, mob->status.def_ele); return;
        case MOB_MODE:    ret_int(info, mob->status.mode); return;
        case MOB_ID:      ret_int(info, mob->id); return;
        default: ret_int(info, -1);
    }
}
void WorldHost::getMobDrops_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     {
    int cls = int_arg(info, 0);
    auto* iso_ = iso(info);
    auto cx = ctx(info);
    auto out = v8::Array::New(iso_);
    if (!mobdb_checkid(cls)) { info.GetReturnValue().Set(out); return; }
    auto mob = mob_db.find(cls);
    if (!mob) { info.GetReturnValue().Set(out); return; }
    uint32 idx_out = 0;
    for (int i = 0; i < MAX_MOB_DROP_TOTAL; ++i) {
        if (mob->dropitem[i].nameid == 0) continue;
        if (!item_db.exists(mob->dropitem[i].nameid)) continue;
        auto row = v8::Object::New(iso_);
        (void)row->Set(cx, v8::String::NewFromUtf8(iso_, "itemId").ToLocalChecked(),
            v8::Integer::NewFromUnsigned(iso_, mob->dropitem[i].nameid));
        (void)row->Set(cx, v8::String::NewFromUtf8(iso_, "rate").ToLocalChecked(),
            v8::Integer::New(iso_, mob->dropitem[i].rate));
        (void)out->Set(cx, idx_out++, row);
    }
    info.GetReturnValue().Set(out);
}
void WorldHost::mobInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info)         {
    int cls = int_arg(info, 0);
    int type = int_arg(info, 1, 1);
    if (!mobdb_checkid(cls)) { ret_str(info, ""); return; }
    auto mob = mob_db.find(cls);
    if (!mob) { ret_str(info, ""); return; }
    switch (type) {
        case 1: ret_str(info, mob->name.c_str());  return;
        case 2: ret_str(info, mob->jname.c_str()); return;
        case 3: ret_str(info, ""); return; // sprite — not in db
        default: ret_str(info, "");
    }
}

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
void WorldHost::setUnitName_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    int gid = int_arg(info, 0);
    auto name = str_arg(info, 1);
    auto* bl = map_id2bl(gid);
    if (!bl) return;
    switch (bl->type) {
        case BL_MOB: { auto* md = map_id2md(gid); if (md) safestrncpy(md->name, name.c_str(), NAME_LENGTH); break; }
        case BL_HOM: { auto* hd = map_id2hd(gid); if (hd) safestrncpy(hd->homunculus.name, name.c_str(), NAME_LENGTH); break; }
        case BL_PET: { auto* pd = map_id2pd(gid); if (pd) safestrncpy(pd->pet.name, name.c_str(), NAME_LENGTH); break; }
        default: return;
    }
    clif_name_area(bl);
}
void WorldHost::getUnitTitle_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { ret_str(info, ""); }
void WorldHost::setUnitTitle_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = int_arg(info, 0);
    auto title = str_arg(info, 1);
    auto* bl = map_id2bl(gid);
    if (!bl) return;
    auto* ud = unit_bl2ud(bl);
    if (!ud) return;
    safestrncpy(ud->title, title.c_str(), NAME_LENGTH);
    clif_name_area(bl);
}
void WorldHost::getUnitData_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { ret_null(info); }
void WorldHost::setUnitData_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    int gid = int_arg(info, 0);
    int type = int_arg(info, 1);
    int value = int_arg(info, 2);
    auto* bl = map_id2bl(gid);
    if (!bl || bl->type != BL_MOB) return;
    auto* md = reinterpret_cast<mob_data*>(bl);
    if (!md->base_status) {
        md->base_status = static_cast<status_data*>(aCalloc(1, sizeof(status_data)));
        memcpy(md->base_status, &md->db->status, sizeof(status_data));
    }
    // Cover the most common MOB type codes. The full multiplexer mirrors
    // 700 LOC across MOB/HOM/PET/MER/ELE/NPC types — this is the slice
    // that's actually exercised by typical scripts.
    switch (type) {
        case UMOB_SIZE:    md->status.size = md->base_status->size = static_cast<unsigned char>(value); break;
        case UMOB_LEVEL:   md->level = static_cast<uint16>(value); clif_name_area(&md->bl); break;
        case UMOB_HP:      md->base_status->hp = value; status_set_hp(bl, value, 0); clif_name_area(&md->bl); break;
        case UMOB_MAXHP:   md->base_status->hp = md->base_status->max_hp = value; status_set_maxhp(bl, value, 0); clif_name_area(&md->bl); break;
        case UMOB_X:       unit_movepos(bl, static_cast<int16>(value), md->bl.y, 0, 0); break;
        case UMOB_Y:       unit_movepos(bl, md->bl.x, static_cast<int16>(value), 0, 0); break;
        case UMOB_SPEED:   md->base_status->speed = static_cast<uint16>(value); status_calc_misc(bl, &md->status, md->level); break;
        case UMOB_MODE:    md->base_status->mode = static_cast<e_mode>(value); unit_refresh(bl); break;
        case UMOB_CLASS:   status_set_viewdata(bl, static_cast<uint16>(value)); unit_refresh(bl); break;
        case UMOB_LOOKDIR: unit_setdir(bl, static_cast<uint8>(value)); break;
        case UMOB_MASTERAID: md->master_id = value; break;
        case UMOB_DMGIMMUNE: md->ud.immune_attack = value > 0; break;
        case UMOB_STR: md->base_status->str = static_cast<uint16>(value); status_calc_misc(bl, &md->status, md->level); break;
        case UMOB_AGI: md->base_status->agi = static_cast<uint16>(value); status_calc_misc(bl, &md->status, md->level); break;
        case UMOB_VIT: md->base_status->vit = static_cast<uint16>(value); status_calc_misc(bl, &md->status, md->level); break;
        case UMOB_INT: md->base_status->int_ = static_cast<uint16>(value); status_calc_misc(bl, &md->status, md->level); break;
        case UMOB_DEX: md->base_status->dex = static_cast<uint16>(value); status_calc_misc(bl, &md->status, md->level); break;
        case UMOB_LUK: md->base_status->luk = static_cast<uint16>(value); status_calc_misc(bl, &md->status, md->level); break;
        case UMOB_ATKMIN: md->base_status->rhw.atk  = static_cast<uint16>(value); break;
        case UMOB_ATKMAX: md->base_status->rhw.atk2 = static_cast<uint16>(value); break;
        case UMOB_DEF:    md->base_status->def     = static_cast<defType>(value); break;
        case UMOB_MDEF:   md->base_status->mdef    = static_cast<defType>(value); break;
        case UMOB_HIT:    md->base_status->hit     = static_cast<int16>(value); break;
        case UMOB_FLEE:   md->base_status->flee    = static_cast<int16>(value); break;
        case UMOB_CRIT:   md->base_status->cri     = static_cast<int16>(value); break;
        case UMOB_AMOTION: md->base_status->amotion = static_cast<int16>(value); break;
        case UMOB_ADELAY:  md->base_status->adelay  = static_cast<int16>(value); break;
        case UMOB_DMOTION: md->base_status->dmotion = static_cast<int16>(value); break;
        default: break;
    }
}
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
void WorldHost::setCell_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    {
    auto mname = str_arg(info, 0);
    int x1 = int_arg(info, 1), y1 = int_arg(info, 2);
    int x2 = int_arg(info, 3), y2 = int_arg(info, 4);
    int type = int_arg(info, 5);
    bool flag = bool_arg(info, 6);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m < 0) return;
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    for (int y = y1; y <= y2; ++y)
        for (int x = x1; x <= x2; ++x)
            map_setcell(m, x, y, static_cast<cell_t>(type), flag);
}
void WorldHost::checkCell_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    auto mname = str_arg(info, 0);
    int x = int_arg(info, 1), y = int_arg(info, 2);
    int type = int_arg(info, 3);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m < 0) { ret_int(info, 0); return; }
    ret_int(info, map_getcell(m, x, y, static_cast<cell_chk>(type)));
}
void WorldHost::getFreeCell_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ ret_null(info); }
void WorldHost::setWall_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    {
    auto mname = str_arg(info, 0);
    int x = int_arg(info, 1), y = int_arg(info, 2);
    int size = int_arg(info, 3, 1);
    int dir  = int_arg(info, 4, 0);
    bool shootable = bool_arg(info, 5);
    auto name = str_arg(info, 6);
    int16 m = map_mapname2mapid(mname.c_str());
    if (m < 0) return;
    map_iwall_set(m, x, y, size, dir, shootable, name.c_str());
}
void WorldHost::delWall_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    {
    auto name = str_arg(info, 0);
    map_iwall_remove(name.c_str());
}
void WorldHost::checkWall_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    auto name = str_arg(info, 0);
    ret_bool(info, map_iwall_exist(name.c_str()));
}
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
namespace {
int32 clearfloor_sub(struct block_list* bl, va_list ap) {
    (void)ap;
    if (bl) map_clearflooritem(bl);
    return 0;
}
int32 areapercentheal_sub(struct block_list* bl, va_list ap) {
    int hp = va_arg(ap, int);
    int sp = va_arg(ap, int);
    if (bl && bl->type == BL_PC)
        pc_percentheal(reinterpret_cast<map_session_data*>(bl), hp, sp);
    return 0;
}
int32 mapwarp_sub(struct block_list* bl, va_list ap) {
    int mapindex = va_arg(ap, int);
    int x = va_arg(ap, int);
    int y = va_arg(ap, int);
    if (bl && bl->type == BL_PC)
        pc_setpos(reinterpret_cast<map_session_data*>(bl),
                  static_cast<uint16>(mapindex), x, y, CLR_TELEPORT);
    return 0;
}
} // namespace

void WorldHost::cleanArea_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    auto mname = str_arg(info, 0);
    int x0 = int_arg(info, 1), y0 = int_arg(info, 2);
    int x1 = int_arg(info, 3), y1 = int_arg(info, 4);
    int m = map_mapname2mapid(mname.c_str());
    if (m < 0) return;
    map_foreachinallarea(clearfloor_sub, m, x0, y0, x1, y1, BL_ITEM);
}
void WorldHost::cleanMap_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   {
    auto mname = str_arg(info, 0);
    int m = map_mapname2mapid(mname.c_str());
    if (m < 0) return;
    map_foreachinmap(clearfloor_sub, m, BL_ITEM);
}
void WorldHost::warpPortal_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP_REQ;
    int spx = int_arg(info, 0), spy = int_arg(info, 1);
    auto dest = str_arg(info, 2);
    int tpx = int_arg(info, 3), tpy = int_arg(info, 4);
    int mapindex = mapindex_name2id(dest.c_str());
    if (!mapindex) return;
    auto* nd_bl = map_id2bl(sd->npc_id);
    if (!nd_bl) return;
    auto group = skill_unitsetting(nd_bl, AL_WARP, 4, spx, spy, 0);
    if (!group) return;
    group->val1 = (group->val1 << 16) | static_cast<int16>(0);
    group->val2 = (tpx << 16) | tpy;
    group->val3 = mapindex;
}
void WorldHost::mapWarp_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    {
    auto src = str_arg(info, 0);
    auto dst = str_arg(info, 1);
    int x = int_arg(info, 2), y = int_arg(info, 3);
    int m = map_mapname2mapid(src.c_str());
    int idx = mapindex_name2id(dst.c_str());
    if (m < 0 || !idx) return;
    map_foreachinmap(mapwarp_sub, m, BL_PC, idx, x, y);
}
void WorldHost::areaWarp_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   {
    auto src = str_arg(info, 0);
    int x0 = int_arg(info, 1), y0 = int_arg(info, 2);
    int x1 = int_arg(info, 3), y1 = int_arg(info, 4);
    auto dst = str_arg(info, 5);
    int x = int_arg(info, 6), y = int_arg(info, 7);
    int m = map_mapname2mapid(src.c_str());
    int idx = mapindex_name2id(dst.c_str());
    if (m < 0 || !idx) return;
    map_foreachinallarea(mapwarp_sub, m, x0, y0, x1, y1, BL_PC, idx, x, y);
}
void WorldHost::warpParty_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    int pid = int_arg(info, 0);
    auto dst = str_arg(info, 1);
    int x = int_arg(info, 2), y = int_arg(info, 3);
    auto* p = party_search(pid);
    int idx = mapindex_name2id(dst.c_str());
    if (!p || !idx) return;
    for (int i = 0; i < MAX_PARTY; ++i) {
        auto* pl = p->data[i].sd;
        if (pl) pc_setpos(pl, static_cast<uint16>(idx), x, y, CLR_TELEPORT);
    }
}
void WorldHost::warpGuild_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    int gid = int_arg(info, 0);
    auto dst = str_arg(info, 1);
    int x = int_arg(info, 2), y = int_arg(info, 3);
    auto g = guild_search(gid);
    int idx = mapindex_name2id(dst.c_str());
    if (!g || !idx) return;
    for (int i = 0; i < g->guild.max_member; ++i) {
        auto* pl = g->guild.member[i].sd;
        if (pl) pc_setpos(pl, static_cast<uint16>(idx), x, y, CLR_TELEPORT);
    }
}
void WorldHost::areaPercentHeal_cb(const v8::FunctionCallbackInfo<v8::Value>& info){
    auto mname = str_arg(info, 0);
    int x0 = int_arg(info, 1), y0 = int_arg(info, 2);
    int x1 = int_arg(info, 3), y1 = int_arg(info, 4);
    int hp = int_arg(info, 5), sp = int_arg(info, 6);
    int m = map_mapname2mapid(mname.c_str());
    if (m < 0) return;
    map_foreachinallarea(areapercentheal_sub, m, x0, y0, x1, y1, BL_PC, hp, sp);
}
// attachRid / addRid: in the legacy engine these swap the player a script
// is executing as, then continue inline. The V8 engine works per-callback
// with sd_ scoped to that invocation, so there's no analogous "swap and
// keep running" semantics. Use `ctx.player.<op>(target_account_id)` or
// invoke the handler again with the new rid instead.
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
void WorldHost::setMapFlagNoSave_cb(const v8::FunctionCallbackInfo<v8::Value>& info){
    auto src  = str_arg(info, 0);
    auto dest = str_arg(info, 1);
    int x = int_arg(info, 2);
    int y = int_arg(info, 3);
    int16 m = map_mapname2mapid(src.c_str());
    int dest_index = mapindex_name2id(dest.c_str());
    if (m < 0 || !dest_index) return;
    union u_mapflag_args u{};
    u.nosave.map = static_cast<uint16>(dest_index);
    u.nosave.x = static_cast<int16>(x);
    u.nosave.y = static_cast<int16>(y);
    map_setmapflag_sub(m, MF_NOSAVE, true, &u);
}

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
void WorldHost::flagEmblem_cb(const v8::FunctionCallbackInfo<v8::Value>& info){
    auto npc_name = str_arg(info, 0);
    int gid = int_arg(info, 1);
    auto* nd = npc_name2id(npc_name.c_str());
    if (!nd || nd->subtype != NPCTYPE_SCRIPT || gid < 0) return;
    bool changed = (nd->u.scr.guild_id != gid);
    nd->u.scr.guild_id = gid;
    clif_guild_emblem_area(&nd->bl);
    if (gid)         guild_flag_add(nd);
    else if (changed) guild_flag_remove(nd);
}
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
void WorldHost::bindAtCommand_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  {
    auto cmd = str_arg(info, 0);
    auto event = str_arg(info, 1);
    int level  = int_arg(info, 2, 0);
    int level2 = int_arg(info, 3, 100);
    const char* c = cmd.c_str();
    if (*c == atcommand_symbol || *c == charcommand_symbol) ++c;
    // Look up existing
    int i = 0;
    for (; i < atcmd_binding_count; ++i)
        if (strcmp(atcmd_binding[i]->command, c) == 0) break;
    if (i < atcmd_binding_count) {
        safestrncpy(atcmd_binding[i]->npc_event, event.c_str(), EVENT_NAME_LENGTH);
        atcmd_binding[i]->level  = level;
        atcmd_binding[i]->level2 = level2;
        return;
    }
    if (atcmd_binding_count == 0) {
        CREATE(atcmd_binding, struct atcmd_binding_data*, 1);
    } else {
        RECREATE(atcmd_binding, struct atcmd_binding_data*, atcmd_binding_count + 1);
    }
    i = atcmd_binding_count++;
    CREATE(atcmd_binding[i], struct atcmd_binding_data, 1);
    safestrncpy(atcmd_binding[i]->command,   c,             50);
    safestrncpy(atcmd_binding[i]->npc_event, event.c_str(), EVENT_NAME_LENGTH);
    atcmd_binding[i]->level  = level;
    atcmd_binding[i]->level2 = level2;
}
void WorldHost::unbindAtCommand_cb(const v8::FunctionCallbackInfo<v8::Value>& info){
    auto cmd = str_arg(info, 0);
    const char* c = cmd.c_str();
    if (*c == atcommand_symbol || *c == charcommand_symbol) ++c;
    if (atcmd_binding_count == 0) return;
    int i = 0;
    for (; i < atcmd_binding_count; ++i)
        if (strcmp(atcmd_binding[i]->command, c) == 0) break;
    if (i >= atcmd_binding_count) return;
    aFree(atcmd_binding[i]);
    atcmd_binding[i] = nullptr;
    int cursor = 0;
    for (int j = 0; j < atcmd_binding_count; ++j) {
        if (!atcmd_binding[j]) continue;
        if (cursor != j) atcmd_binding[cursor] = atcmd_binding[j];
        ++cursor;
    }
    atcmd_binding_count = cursor;
}

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
void WorldHost::setItemInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   {
    int item_id = int_arg(info, 0);
    int type    = int_arg(info, 1);
    int value   = int_arg(info, 2);
    auto i_data = item_db.find(item_id);
    if (!i_data) { ret_int(info, -1); return; }
    switch (type) {
        case ITEMINFO_BUY:           i_data->value_buy  = value; break;
        case ITEMINFO_SELL:          i_data->value_sell = value; break;
        case ITEMINFO_TYPE:          i_data->type    = static_cast<item_types>(value); break;
        case ITEMINFO_MAXCHANCE:     i_data->maxchance = value; break;
        case ITEMINFO_GENDER:        i_data->sex     = static_cast<uint8>(value); break;
        case ITEMINFO_LOCATIONS:     i_data->equip   = value; break;
        case ITEMINFO_WEIGHT:        i_data->weight  = value; break;
        case ITEMINFO_ATTACK:        i_data->atk     = value; break;
        case ITEMINFO_DEFENSE:       i_data->def     = value; break;
        case ITEMINFO_RANGE:         i_data->range   = static_cast<uint16>(value); break;
        case ITEMINFO_SLOT:          i_data->slots   = static_cast<uint16>(value); break;
        case ITEMINFO_EQUIPLEVELMIN: i_data->elv     = static_cast<uint16>(value); break;
        case ITEMINFO_EQUIPLEVELMAX: i_data->elvmax  = static_cast<uint16>(value); break;
        case ITEMINFO_SUBTYPE:       i_data->subtype = static_cast<uint8>(value); break;
        default: ret_int(info, -1); return;
    }
    ret_int(info, value);
}
void WorldHost::setItemScript_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    t_itemid item_id = static_cast<t_itemid>(uint_arg(info, 0));
    auto src = str_arg(info, 1);
    int slot = int_arg(info, 2, 0);
    auto i_data = item_db.find(item_id);
    if (!i_data) { ret_int(info, 0); return; }
    if (!src.empty() && src[0] != '{') { ret_int(info, 0); return; }
    struct script_code** dst;
    switch (slot) {
        case 2: dst = &i_data->unequip_script; break;
        case 1: dst = &i_data->equip_script;   break;
        default: dst = &i_data->script;        break;
    }
    if (*dst) script_free_code(*dst);
    *dst = src.empty() ? nullptr : parse_script(src.c_str(), "ts_setitemscript", 0, 0);
    ret_int(info, 1);
}
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
