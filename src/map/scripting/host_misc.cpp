#ifdef HAVE_TS_SCRIPTING

#include "host_misc.hpp"

#include "arg_helpers.hpp"
#include "../achievement.hpp"
#include "../channel.hpp"
#include "../chat.hpp"
#include "../clif.hpp"
#include "../guild.hpp"
#include "../instance.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../homunculus.hpp"
#include "../mail.hpp"
#include "../battle.hpp"
#include "../battleground.hpp"
#include "../intif.hpp"
#include "../mercenary.hpp"
#include "../mob.hpp"
#include "../party.hpp"
#include "../pc.hpp"
#include "../pet.hpp"
#include "../quest.hpp"
#include "../script.hpp"
#include "../log.hpp"
#include "../skill.hpp"
#include "../status.hpp"
#include "../storage.hpp"
#include "../unit.hpp"
#include "../../common/showmsg.hpp"
#include "../../common/timer.hpp"
#include "../../common/utils.hpp"

namespace rathena::scripting {

namespace {
using namespace args;

// ---- Shared generic placeholders ------------------------------------------
//
// Every method on the sub-objects below needs to exist as a real C++
// callback. The vast majority are wiring waiting on a deeper rAthena
// subsystem — until then they install one of these type-correct
// no-ops. NO call falls back to the generic stub_callback; every
// host owns every method name on its target object.

void void_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { (void)info; }

void int0_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { ret_int(info, 0); }
void int1_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { ret_int(info, 1); }

void str_empty_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { ret_str(info, ""); }
void false_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     { ret_bool(info, false); }
void true_cb(const v8::FunctionCallbackInfo<v8::Value>& info)      { ret_bool(info, true); }
void null_cb(const v8::FunctionCallbackInfo<v8::Value>& info)      { ret_null(info); }
void empty_arr_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().Set(v8::Array::New(iso(info), 0));
}
void empty_obj_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().Set(v8::Object::New(iso(info)));
}

// ---- bind helpers — assume `obj` + `iso`/`ctx` are in scope --------------
//
// These wrap args::bind_method with the right shared callback. They
// don't carry a host pointer (the placeholder callbacks don't need
// `this`), so we pass nullptr as the "self" via a small specialization.

inline void bind_void(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                      v8::Local<v8::Object> obj, const char* name) {
    auto data = v8::External::New(iso, nullptr, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, &void_cb, data)
        ->GetFunction(ctx).ToLocalChecked();
    (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
}
inline void bind_int0(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                      v8::Local<v8::Object> obj, const char* name) {
    auto data = v8::External::New(iso, nullptr, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, &int0_cb, data)
        ->GetFunction(ctx).ToLocalChecked();
    (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
}
inline void bind_int1(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                      v8::Local<v8::Object> obj, const char* name) {
    auto data = v8::External::New(iso, nullptr, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, &int1_cb, data)
        ->GetFunction(ctx).ToLocalChecked();
    (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
}
inline void bind_str(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                     v8::Local<v8::Object> obj, const char* name) {
    auto data = v8::External::New(iso, nullptr, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, &str_empty_cb, data)
        ->GetFunction(ctx).ToLocalChecked();
    (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
}
inline void bind_false(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                       v8::Local<v8::Object> obj, const char* name) {
    auto data = v8::External::New(iso, nullptr, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, &false_cb, data)
        ->GetFunction(ctx).ToLocalChecked();
    (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
}
inline void bind_true(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                      v8::Local<v8::Object> obj, const char* name) {
    auto data = v8::External::New(iso, nullptr, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, &true_cb, data)
        ->GetFunction(ctx).ToLocalChecked();
    (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
}
inline void bind_null(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                      v8::Local<v8::Object> obj, const char* name) {
    auto data = v8::External::New(iso, nullptr, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, &null_cb, data)
        ->GetFunction(ctx).ToLocalChecked();
    (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
}
inline void bind_arr(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                     v8::Local<v8::Object> obj, const char* name) {
    auto data = v8::External::New(iso, nullptr, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, &empty_arr_cb, data)
        ->GetFunction(ctx).ToLocalChecked();
    (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
}
inline void bind_obj_ret(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                         v8::Local<v8::Object> obj, const char* name) {
    auto data = v8::External::New(iso, nullptr, v8::kExternalPointerTypeTagDefault);
    auto fn = v8::FunctionTemplate::New(iso, &empty_obj_cb, data)
        ->GetFunction(ctx).ToLocalChecked();
    (void)obj->Set(ctx, v8::String::NewFromUtf8(iso, name).ToLocalChecked(), fn);
}

} // namespace

// =====================================================================
// NpcInfoHost (ctx.npc)
// =====================================================================
//
// A handful of npc display ops have real implementations; the rest
// are placeholders waiting on npc.cpp helper exposure.

namespace {

// Real impl: hide / show / enable / disable have direct npc.cpp helpers.
void npc_hide_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    npc_enable_target(self->nd(), 0, NPCVIEW_HIDEON);
}
void npc_show_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    npc_enable_target(self->nd(), 0, NPCVIEW_HIDEOFF);
}
void npc_disable_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    npc_enable_target(self->nd(), 0, NPCVIEW_DISABLE);
}
void npc_enable_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    npc_enable_target(self->nd(), 0, NPCVIEW_ENABLE);
}

} // namespace

namespace {
void npc_setDisplay_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    auto name = args::str_arg(info, 0);
    npc_setdisplayname(&self->nd(), name.c_str());
    if (args::has(info, 1)) npc_setclass(&self->nd(), args::int_arg(info, 1));
}
void npc_speed_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    self->nd().speed = args::int_arg(info, 0, DEFAULT_NPC_WALK_SPEED);
}
void npc_walkTo_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    unit_walktoxy(&self->nd().bl, args::int_arg(info, 0), args::int_arg(info, 1), 0);
}
void npc_stop_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    unit_stop_walking(&self->nd().bl, args::bool_arg(info, 0) ? 1 : 0);
}
void npc_moveTo_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    int x = args::int_arg(info, 0), y = args::int_arg(info, 1);
    int dir = args::int_arg(info, 2, self->nd().ud.dir);
    unit_warp(&self->nd().bl, self->nd().bl.m, x, y, CLR_TELEPORT);
    self->nd().ud.dir = static_cast<uint8>(dir);
}

chat_data* npc_cd(NpcInfoHost* self) {
    if (!self) return nullptr;
    return reinterpret_cast<chat_data*>(map_id2bl(self->nd().chat_id));
}

void npc_createWaitingRoom_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (!self) return;
    auto title  = args::str_arg(info, 0);
    int limit   = args::int_arg(info, 1);
    auto ev     = args::str_arg(info, 2);
    int trigger = args::int_arg(info, 3, limit);
    int zeny    = args::int_arg(info, 4, 0);
    int minLvl  = args::int_arg(info, 5, 1);
    int maxLvl  = args::int_arg(info, 6, MAX_LEVEL);
    chat_createnpcchat(&self->nd(), title.c_str(), limit, 1,
                       trigger, ev.c_str(), zeny, minLvl, maxLvl);
}
void npc_removeWaitingRoom_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<NpcInfoHost>(info);
    if (self) chat_deletenpcchat(&self->nd());
}
void npc_enableWaitingRoom_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* cd = npc_cd(args::unwrap<NpcInfoHost>(info));
    if (cd) chat_enableevent(cd);
}
void npc_disableWaitingRoom_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* cd = npc_cd(args::unwrap<NpcInfoHost>(info));
    if (cd) chat_disableevent(cd);
}
void npc_kickWaitingRoomUser_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* cd = npc_cd(args::unwrap<NpcInfoHost>(info));
    if (!cd) return;
    auto user = args::str_arg(info, 0);
    chat_npckickchat(cd, user.c_str());
}
void npc_kickAllWaitingRoom_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* cd = npc_cd(args::unwrap<NpcInfoHost>(info));
    if (cd) chat_npckickall(cd);
}
void npc_getWaitingRoomUsers_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* cd = npc_cd(args::unwrap<NpcInfoHost>(info));
    args::ret_int(info, cd ? cd->users : 0);
}
void npc_getWaitingRoomState_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* cd = npc_cd(args::unwrap<NpcInfoHost>(info));
    if (!cd) { args::ret_int(info, -1); return; }
    int type = args::int_arg(info, 0);
    switch (type) {
        case 0:  args::ret_int(info, cd->users); return;
        case 1:  args::ret_int(info, cd->limit); return;
        case 2:  args::ret_int(info, cd->trigger & 0x7f); return;
        case 3:  args::ret_int(info, (cd->trigger & 0x80) != 0); return;
        case 32: args::ret_int(info, cd->users >= cd->limit); return;
        case 33: args::ret_int(info, cd->users >= cd->trigger); return;
        default: args::ret_int(info, -1);
    }
}
} // namespace

void NpcInfoHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                    v8::Local<v8::Object> obj) {
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "hide",       &npc_hide_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "show",       &npc_show_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "disable",    &npc_disable_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "enable",     &npc_enable_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "setDisplay", &npc_setDisplay_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "speed",      &npc_speed_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "walkTo",     &npc_walkTo_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "stop",       &npc_stop_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "moveTo",     &npc_moveTo_cb);
    bind_void(iso, ctx, obj, "duplicateDynamic");

    // Shop
    bind_void(iso, ctx, obj, "shopSet");
    bind_void(iso, ctx, obj, "shopAdd");
    bind_void(iso, ctx, obj, "shopDel");
    bind_void(iso, ctx, obj, "shopAttach");
    bind_void(iso, ctx, obj, "shopUpdate");

    // Waiting room
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "createWaitingRoom",   &npc_createWaitingRoom_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "removeWaitingRoom",   &npc_removeWaitingRoom_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "enableWaitingRoom",   &npc_enableWaitingRoom_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "disableWaitingRoom",  &npc_disableWaitingRoom_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "getWaitingRoomState", &npc_getWaitingRoomState_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "kickWaitingRoomUser", &npc_kickWaitingRoomUser_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "kickAllWaitingRoom",  &npc_kickAllWaitingRoom_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "getWaitingRoomUsers", &npc_getWaitingRoomUsers_cb);
    // warpWaitingPc body lives mostly in script.cpp w/ NPC->oid lookup —
    // leave as a placeholder until we move that into chat.cpp.
    bind_void(iso, ctx, obj, "warpWaitingPc");

    bind_int0(iso, ctx, obj, "npcTimer");
    bind_int0(iso, ctx, obj, "getNpcId");
    bind_str (iso, ctx, obj, "npcInfo");
}

// =====================================================================
// Quest / Achievement / Storage / Cart / Mail / Pet / Hom / Merc
// =====================================================================
//
// One install_on_object per sub-host. Methods are bound to type-correct
// placeholder callbacks. Promote individual ones by replacing the
// bind_X line with a real custom callback bound via bind_method<Cls>.

namespace {
void quest_add_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<QuestHost>(info);
    if (!self) return;
    quest_add(&self->sd(), args::int_arg(info, 0));
}
void quest_complete_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<QuestHost>(info);
    if (!self) return;
    quest_update_status(&self->sd(), args::int_arg(info, 0), Q_COMPLETE);
}
void quest_erase_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<QuestHost>(info);
    if (!self) return;
    quest_delete(&self->sd(), args::int_arg(info, 0));
}
void quest_change_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<QuestHost>(info);
    if (!self) return;
    quest_change(&self->sd(), args::int_arg(info, 0), args::int_arg(info, 1));
}
void quest_check_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<QuestHost>(info);
    if (!self) { args::ret_int(info, 0); return; }
    auto mode = args::str_arg(info, 1, "any");
    e_quest_check_type t = HAVEQUEST;
    if (mode == "playtime") t = PLAYTIME;
    else if (mode == "hunting") t = HUNTING;
    args::ret_int(info, quest_check(&self->sd(), args::int_arg(info, 0), t));
}
void quest_isBegin_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<QuestHost>(info);
    if (!self) { args::ret_bool(info, false); return; }
    args::ret_bool(info, quest_check(&self->sd(), args::int_arg(info, 0), HAVEQUEST) == Q_ACTIVE);
}
void quest_showEvent_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<QuestHost>(info);
    if (!self) return;
    int icon  = args::int_arg(info, 0);
    int color = args::int_arg(info, 1, QMARK_NONE);
    if (color < QMARK_NONE || color >= QMARK_MAX) color = QMARK_NONE;
#if PACKETVER >= 20120410
    if (icon < 0 || (icon > 8 && icon != QTYPE_NONE) || icon == 7)
        icon = QTYPE_NONE;
#else
    if (icon < 0 || icon > 7) icon = 0;
    else icon += 1;
#endif
    clif_quest_show_event(&self->sd(), &self->sd().bl,
        static_cast<e_questinfo_types>(icon),
        static_cast<e_questinfo_markcolor>(color));
}
void quest_refreshInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<QuestHost>(info);
    if (!self) return;
    pc_show_questinfo(&self->sd());
}
} // namespace

void QuestHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                  v8::Local<v8::Object> obj) {
    args::bind_method<QuestHost>(iso, ctx, obj, this, "add",         &quest_add_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "complete",    &quest_complete_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "erase",       &quest_erase_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "change",      &quest_change_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "check",       &quest_check_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "isBegin",     &quest_isBegin_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "showEvent",   &quest_showEvent_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "refreshInfo", &quest_refreshInfo_cb);
    // showInfo is the area-marker variant requiring a registerquest cond
    // — placeholder for now (uses pc_show_questinfo too in practice).
    args::bind_method<QuestHost>(iso, ctx, obj, this, "showInfo",    &quest_refreshInfo_cb);
}

namespace {
void ach_add_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<AchievementHost>(info);
    if (!self) return;
    achievement_add(&self->sd(), args::int_arg(info, 0));
}
void ach_remove_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<AchievementHost>(info);
    if (!self) return;
    achievement_remove(&self->sd(), args::int_arg(info, 0));
}
void ach_exists_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<AchievementHost>(info);
    if (!self) { args::ret_bool(info, false); return; }
    int id = args::int_arg(info, 0);
    auto& ad = self->sd().achievement_data;
    for (uint16 i = 0; i < ad.count; ++i) {
        if (ad.achievements[i].achievement_id == id) { args::ret_bool(info, true); return; }
    }
    args::ret_bool(info, false);
}
void ach_info_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<AchievementHost>(info);
    if (!self) { args::ret_int(info, 0); return; }
    int id = args::int_arg(info, 0);
    int type = args::int_arg(info, 1);
    args::ret_int(info, achievement_check_progress(&self->sd(), id, type));
}
void ach_complete_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<AchievementHost>(info);
    if (!self) { args::ret_bool(info, false); return; }
    int id = args::int_arg(info, 0);
    if (!achievement_db.exists(id)) { args::ret_bool(info, false); return; }
    if (!self->sd().state.pc_loaded) { args::ret_bool(info, false); return; }
    auto& ad = self->sd().achievement_data;
    uint16 i = 0;
    for (; i < ad.count; ++i) {
        if (ad.achievements[i].achievement_id == id) break;
    }
    if (i == ad.count) achievement_add(&self->sd(), id);
    achievement_update_achievement(&self->sd(), id, true);
    args::ret_bool(info, true);
}
void ach_update_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<AchievementHost>(info);
    if (!self) { args::ret_bool(info, false); return; }
    int id    = args::int_arg(info, 0);
    int type  = args::int_arg(info, 1);
    int value = args::int_arg(info, 2);
    if (!achievement_db.exists(id)) { args::ret_bool(info, false); return; }
    if (!self->sd().state.pc_loaded) { args::ret_bool(info, false); return; }
    auto& ad = self->sd().achievement_data;
    uint16 i = 0;
    for (; i < ad.count; ++i)
        if (ad.achievements[i].achievement_id == id) break;
    if (i == ad.count) {
        achievement_add(&self->sd(), id);
        for (i = 0; i < ad.count; ++i)
            if (ad.achievements[i].achievement_id == id) break;
        if (i == ad.count) { args::ret_bool(info, false); return; }
    }
    if (type >= ACHIEVEINFO_COUNT1 && type <= ACHIEVEINFO_COUNT10)
        ad.achievements[i].count[type - 1] = value;
    else if (type == ACHIEVEINFO_COMPLETE || type == ACHIEVEINFO_COMPLETEDATE)
        ad.achievements[i].completed = value;
    else if (type == ACHIEVEINFO_GOTREWARD)
        ad.achievements[i].rewarded = value;
    else { args::ret_bool(info, false); return; }
    achievement_update_achievement(&self->sd(), id, false);
    args::ret_bool(info, true);
}
} // namespace

void AchievementHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                        v8::Local<v8::Object> obj) {
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "add",      &ach_add_cb);
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "remove",   &ach_remove_cb);
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "exists",   &ach_exists_cb);
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "info",     &ach_info_cb);
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "complete", &ach_complete_cb);
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "update",   &ach_update_cb);
}

namespace {
void storage_open_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<StorageHost>(info);
    if (!self) return;
    storage_storageopen(&self->sd());
}
void storage_openGuild_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<StorageHost>(info);
    if (!self) return;
    storage_guild_storageopen(&self->sd());
}
void storage_countItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<StorageHost>(info);
    if (!self) { args::ret_int(info, 0); return; }
    t_itemid id = static_cast<t_itemid>(args::uint_arg(info, 0));
    int total = 0;
    for (int i = 0; i < self->sd().storage.max_amount; ++i) {
        if (self->sd().storage.u.items_storage[i].nameid == id)
            total += self->sd().storage.u.items_storage[i].amount;
    }
    args::ret_int(info, total);
}
} // namespace

void StorageHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                    v8::Local<v8::Object> obj) {
    args::bind_method<StorageHost>(iso, ctx, obj, this, "open",             &storage_open_cb);
    args::bind_method<StorageHost>(iso, ctx, obj, this, "openGuildStorage", &storage_openGuild_cb);
    args::bind_method<StorageHost>(iso, ctx, obj, this, "countItem",        &storage_countItem_cb);
    // The rest depend on storage_delitem helpers + extended storage IDs;
    // deferred until needed by an actual script.
    bind_void(iso, ctx, obj, "openExtra");
    bind_void(iso, ctx, obj, "delItem");
    bind_int0(iso, ctx, obj, "countGuildItem");
    bind_void(iso, ctx, obj, "delGuildItem");
    bind_arr (iso, ctx, obj, "guildLog");
}

void CartHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                 v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_false(iso, ctx, obj, "isEnabled");
    bind_int0 (iso, ctx, obj, "countItem");
    bind_void (iso, ctx, obj, "delItem");
}

namespace {
void mail_open_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MailHost>(info);
    if (!self) return;
    clif_Mail_window(self->sd().fd, 0);
}
} // namespace

void MailHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                 v8::Local<v8::Object> obj) {
    args::bind_method<MailHost>(iso, ctx, obj, this, "open", &mail_open_cb);
}

namespace {
void pet_catchPet_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self) return;
    int item_id = args::int_arg(info, 0);
    int flag    = args::int_arg(info, 1, PET_CATCH_NORMAL);
    if (flag < PET_CATCH_NORMAL || flag >= PET_CATCH_MAX)
        flag = PET_CATCH_NORMAL;
    pet_catch_process_start(self->sd(),
        static_cast<t_itemid>(item_id),
        static_cast<e_pet_catch_flag>(flag));
}

void pet_makePet_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self) return;
    uint16 mob_id = static_cast<uint16>(args::int_arg(info, 0));
    auto pet = pet_db.find(mob_id);
    if (!pet) {
        ShowError("PetHost::makePet: no pet entry for mob_id %hu\n", mob_id);
        return;
    }
    auto mdb = mob_db.find(pet->class_);
    intif_create_pet(self->sd().status.account_id,
                     self->sd().status.char_id,
                     pet->class_, mdb->lv, pet->EggID, 0,
                     pet->intimate, 100, 0, 1, mdb->jname.c_str());
}

void pet_birthPet_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self) return;
    if (self->sd().status.pet_id) return;
    clif_sendegg(&self->sd());
}

void pet_openIncubator_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self) return;
    clif_sendegg(&self->sd());
}

void pet_info_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    int type = args::int_arg(info, 0);
    if (!self || !self->sd().pd) {
        if (type == PETINFO_NAME) args::ret_str(info, "null");
        else                       args::ret_int(info, 0);
        return;
    }
    auto* pd = self->sd().pd;
    switch (type) {
        case PETINFO_ID:       args::ret_int(info, pd->pet.pet_id); return;
        case PETINFO_CLASS:    args::ret_int(info, pd->pet.class_); return;
        case PETINFO_NAME:     args::ret_str(info, pd->pet.name);   return;
        case PETINFO_INTIMATE: args::ret_int(info, pd->pet.intimate); return;
        case PETINFO_HUNGRY:   args::ret_int(info, pd->pet.hungry);  return;
        case PETINFO_RENAMED:  args::ret_int(info, pd->pet.rename_flag); return;
        case PETINFO_LEVEL:    args::ret_int(info, pd->pet.level);   return;
        case PETINFO_BLOCKID:  args::ret_int(info, pd->bl.id);       return;
        case PETINFO_EGGID:    args::ret_int(info, pd->pet.egg_id);  return;
        case PETINFO_FOODID:   args::ret_int(info, pd->get_pet_db()->FoodID); return;
        default: args::ret_int(info, 0);
    }
}

void pet_skillBonus_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self || !self->sd().pd) return;
    auto* pd = self->sd().pd;
    if (pd->bonus) {
        if (pd->bonus->timer != INVALID_TIMER)
            delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
    } else {
        pd->bonus = (struct pet_bonus*)aMalloc(sizeof(struct pet_bonus));
    }
    pd->bonus->type     = args::int_arg(info, 0);
    pd->bonus->val      = args::int_arg(info, 1);
    pd->bonus->duration = args::int_arg(info, 2);
    pd->bonus->delay    = args::int_arg(info, 3);
    if (pd->state.skillbonus == 1) pd->state.skillbonus = 0;
    if (battle_config.pet_equip_required && pd->pet.equip == 0)
        pd->bonus->timer = INVALID_TIMER;
    else
        pd->bonus->timer = add_timer(gettick() + pd->bonus->delay * 1000,
                                     pet_skill_bonus_timer,
                                     self->sd().bl.id, 0);
}

void pet_skillSupport_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self || !self->sd().pd) return;
    int32 id = skill_get_index(args::int_arg(info, 0));
    if (!id) return;
    auto* pd = self->sd().pd;
    if (pd->s_skill) {
        if (pd->s_skill->timer != INVALID_TIMER) {
            if (pd->s_skill->id)
                delete_timer(pd->s_skill->timer, pet_skill_support_timer);
            else
                delete_timer(pd->s_skill->timer, pet_heal_timer);
        }
    } else {
        pd->s_skill = (struct pet_skill_support*)aMalloc(sizeof(struct pet_skill_support));
    }
    pd->s_skill->id    = id;
    pd->s_skill->lv    = args::int_arg(info, 1);
    pd->s_skill->delay = args::int_arg(info, 2);
    pd->s_skill->hp    = args::int_arg(info, 3);
    pd->s_skill->sp    = args::int_arg(info, 4);
    if (battle_config.pet_equip_required && pd->pet.equip == 0)
        pd->s_skill->timer = INVALID_TIMER;
    else
        pd->s_skill->timer = add_timer(gettick() + pd->s_skill->delay * 1000,
                                       pet_skill_support_timer,
                                       self->sd().bl.id, 0);
}

void pet_skillAttack_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self || !self->sd().pd) return;
    int32 id = skill_get_index(args::int_arg(info, 0));
    if (!id) return;
    auto* pd = self->sd().pd;
    if (pd->a_skill == nullptr)
        pd->a_skill = (struct pet_skill_attack*)aMalloc(sizeof(struct pet_skill_attack));
    pd->a_skill->id        = id;
    pd->a_skill->damage    = 0;
    pd->a_skill->lv        = static_cast<uint16>(
        std::min(args::int_arg(info, 1),
                 static_cast<int>(skill_get_max(pd->a_skill->id))));
    pd->a_skill->div_      = 0;
    pd->a_skill->rate      = args::int_arg(info, 2);
    pd->a_skill->bonusrate = args::int_arg(info, 3);
}

void pet_skillAttack2_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self || !self->sd().pd) return;
    int32 id = skill_get_index(args::int_arg(info, 0));
    if (!id) return;
    auto* pd = self->sd().pd;
    if (pd->a_skill == nullptr)
        pd->a_skill = (struct pet_skill_attack*)aMalloc(sizeof(struct pet_skill_attack));
    pd->a_skill->id        = id;
    pd->a_skill->damage    = args::int_arg(info, 1);
    pd->a_skill->lv        = static_cast<uint16>(skill_get_max(pd->a_skill->id));
    pd->a_skill->div_      = args::int_arg(info, 2);
    pd->a_skill->rate      = args::int_arg(info, 3);
    pd->a_skill->bonusrate = args::int_arg(info, 4);
}

void pet_recovery_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self || !self->sd().pd) return;
    int sc = args::int_arg(info, 0);
    if (sc <= SC_NONE || sc >= SC_MAX) return;
    auto* pd = self->sd().pd;
    if (pd->recovery) {
        if (pd->recovery->timer != INVALID_TIMER)
            delete_timer(pd->recovery->timer, pet_recovery_timer);
    } else {
        pd->recovery = (struct pet_recovery*)aMalloc(sizeof(struct pet_recovery));
    }
    pd->recovery->type  = static_cast<sc_type>(sc);
    pd->recovery->delay = args::int_arg(info, 1);
    pd->recovery->timer = INVALID_TIMER;
}

void pet_loot_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PetHost>(info);
    if (!self || !self->sd().pd) return;
    int max = args::int_arg(info, 0);
    if (max < 1) max = 1;
    else if (max > MAX_PETLOOT_SIZE) max = MAX_PETLOOT_SIZE;
    auto* pd = self->sd().pd;
    if (pd->loot != nullptr) {
        pet_lootitem_drop(*pd, pd->master);
        aFree(pd->loot->item);
    } else {
        pd->loot = (struct pet_loot*)aMalloc(sizeof(struct pet_loot));
    }
    pd->loot->item   = (struct item*)aCalloc(max, sizeof(struct item));
    pd->loot->max    = max;
    pd->loot->count  = 0;
    pd->loot->weight = 0;
}
} // namespace

void PetHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                v8::Local<v8::Object> obj) {
    args::bind_method<PetHost>(iso, ctx, obj, this, "catchPet",      &pet_catchPet_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "makePet",       &pet_makePet_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "birthPet",      &pet_birthPet_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "openIncubator", &pet_openIncubator_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "info",          &pet_info_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "skillBonus",    &pet_skillBonus_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "skillSupport",  &pet_skillSupport_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "skillAttack",   &pet_skillAttack_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "skillAttack2",  &pet_skillAttack2_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "recovery",      &pet_recovery_cb);
    args::bind_method<PetHost>(iso, ctx, obj, this, "loot",          &pet_loot_cb);
}

namespace {
void hom_exists_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<HomHost>(info);
    args::ret_bool(info, self && self->sd().status.hom_id != 0);
}
void hom_isCalled_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<HomHost>(info);
    args::ret_bool(info, self && self->sd().hd != nullptr && hom_is_active(self->sd().hd));
}
void hom_evolve_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<HomHost>(info);
    if (self && self->sd().hd) hom_evolution(self->sd().hd);
}
void hom_mutate_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<HomHost>(info);
    if (!self || !self->sd().hd) return;
    int id = args::int_arg(info, 0, 0);
    if (id > 0) hom_mutate(self->sd().hd, id);
}
void hom_shuffle_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<HomHost>(info);
    if (self && self->sd().hd) hom_shuffle(self->sd().hd);
}
void hom_info_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<HomHost>(info);
    int type = args::int_arg(info, 0);
    if (!self || !self->sd().hd) {
        if (type == 2) args::ret_str(info, "null");
        else            args::ret_int(info, 0);
        return;
    }
    auto* hd = self->sd().hd;
    switch (type) {
        case 0: args::ret_int(info, hd->homunculus.hom_id); return;
        case 1: args::ret_int(info, hd->homunculus.class_); return;
        case 2: args::ret_str(info, hd->homunculus.name);   return;
        case 3: args::ret_int(info, hd->homunculus.intimacy); return;
        case 4: args::ret_int(info, hd->homunculus.hunger);   return;
        case 5: args::ret_int(info, hd->homunculus.rename_flag); return;
        case 6: args::ret_int(info, hd->homunculus.level);    return;
        case 7: args::ret_int(info, hd->bl.id);               return;
        default: args::ret_int(info, 0);
    }
}
void hom_morph_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<HomHost>(info);
    if (!self || self->sd().hd == nullptr) return;
    auto& sd = self->sd();
    if (!hom_is_active(sd.hd)) { clif_emotion(sd.bl, ET_SWEAT); return; }
    int m_class = hom_class2mapid(sd.hd->homunculus.class_);
    if (m_class == -1 || !(m_class & HOM_EVO) || sd.hd->homunculus.level < 99) {
        clif_emotion(sd.hd->bl, ET_SWEAT); return;
    }
    struct item item_tmp{};
    item_tmp.nameid   = ITEMID_STRANGE_EMBRYO;
    item_tmp.identify = 1;
    char i = pc_additem(&sd, &item_tmp, 1, LOG_TYPE_SCRIPT);
    if (i) {
        clif_additem(&sd, 0, 0, i);
        clif_emotion(sd.bl, ET_SWEAT);
    } else {
        hom_vaporize(&sd, HOM_ST_MORPH);
    }
}
void hom_addIntimacy_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<HomHost>(info);
    if (!self || self->sd().hd == nullptr) return;
    int value = args::int_arg(info, 0);
    if (value == 0) return;
    if (value > 0) hom_increase_intimacy(self->sd().hd, static_cast<uint32>(value));
    else            hom_decrease_intimacy(self->sd().hd, static_cast<uint32>(std::abs(value)));
    clif_send_homdata(*self->sd().hd, SP_INTIMATE);
}
} // namespace

void HomHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                v8::Local<v8::Object> obj) {
    args::bind_method<HomHost>(iso, ctx, obj, this, "exists",      &hom_exists_cb);
    args::bind_method<HomHost>(iso, ctx, obj, this, "isCalled",    &hom_isCalled_cb);
    args::bind_method<HomHost>(iso, ctx, obj, this, "evolve",      &hom_evolve_cb);
    args::bind_method<HomHost>(iso, ctx, obj, this, "mutate",      &hom_mutate_cb);
    args::bind_method<HomHost>(iso, ctx, obj, this, "shuffle",     &hom_shuffle_cb);
    args::bind_method<HomHost>(iso, ctx, obj, this, "info",        &hom_info_cb);
    args::bind_method<HomHost>(iso, ctx, obj, this, "morph",       &hom_morph_cb);
    args::bind_method<HomHost>(iso, ctx, obj, this, "addIntimacy", &hom_addIntimacy_cb);
}

namespace {
int* merc_calls_slot(map_session_data& sd, int guild) {
    switch (guild) {
        case ARCH_MERC_GUILD:  return &sd.status.arch_calls;
        case SPEAR_MERC_GUILD: return &sd.status.spear_calls;
        case SWORD_MERC_GUILD: return &sd.status.sword_calls;
        default: return nullptr;
    }
}
int* merc_faith_slot(map_session_data& sd, int guild) {
    switch (guild) {
        case ARCH_MERC_GUILD:  return &sd.status.arch_faith;
        case SPEAR_MERC_GUILD: return &sd.status.spear_faith;
        case SWORD_MERC_GUILD: return &sd.status.sword_faith;
        default: return nullptr;
    }
}

void merc_create_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MercHost>(info);
    if (!self) return;
    auto& sd = self->sd();
    if (sd.md || sd.status.mer_id != 0) return;
    int class_ = args::int_arg(info, 0);
    if (!mercenary_db.exists(class_)) return;
    uint32 contract_time = static_cast<uint32>(args::int_arg(info, 1));
    mercenary_create(&sd, static_cast<uint16>(class_), contract_time);
}

void merc_delete_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MercHost>(info);
    if (!self || !self->sd().md) return;
    int type = args::int_arg(info, 0, 0);
    if (type < 0 || type > 3) type = 0;
    mercenary_delete(self->sd().md, type);
}

void merc_heal_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MercHost>(info);
    if (!self || !self->sd().md) return;
    int hp = args::int_arg(info, 0);
    int sp = args::int_arg(info, 1, 0);
    status_heal(&self->sd().md->bl, hp, sp, 0);
}

void merc_scStart_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MercHost>(info);
    if (!self || !self->sd().md) return;
    sc_type type = static_cast<sc_type>(args::int_arg(info, 0));
    int tick = args::int_arg(info, 1);
    int val1 = args::int_arg(info, 2);
    status_change_start(nullptr, &self->sd().md->bl, type, 10000,
                        val1, 0, 0, 0, tick, SCSTART_NOTICKDEF);
}

void merc_getCalls_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MercHost>(info);
    if (!self) { args::ret_int(info, 0); return; }
    int* slot = merc_calls_slot(self->sd(), args::int_arg(info, 0));
    args::ret_int(info, slot ? *slot : 0);
}

void merc_setCalls_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MercHost>(info);
    if (!self) return;
    int* slot = merc_calls_slot(self->sd(), args::int_arg(info, 0));
    if (!slot) return;
    *slot += args::int_arg(info, 1);
    *slot = cap_value(*slot, 0, INT_MAX);
}

void merc_getFaith_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MercHost>(info);
    if (!self) { args::ret_int(info, 0); return; }
    int* slot = merc_faith_slot(self->sd(), args::int_arg(info, 0));
    args::ret_int(info, slot ? *slot : 0);
}

void merc_setFaith_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MercHost>(info);
    if (!self) return;
    int guild = args::int_arg(info, 0);
    int* slot = merc_faith_slot(self->sd(), guild);
    if (!slot) return;
    *slot += args::int_arg(info, 1);
    *slot = cap_value(*slot, 0, INT_MAX);
    if (self->sd().md && mercenary_get_guild(self->sd().md) == guild)
        clif_mercenary_updatestatus(&self->sd(), SP_MERCFAITH);
}

void merc_info_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<MercHost>(info);
    int type = args::int_arg(info, 0);
    if (!self) { args::ret_null(info); return; }
    auto* md = self->sd().md;
    if (md == nullptr) {
        if (type == 2) args::ret_str(info, "");
        else            args::ret_int(info, 0);
        return;
    }
    switch (type) {
        case 0: args::ret_int(info, md->mercenary.mercenary_id); return;
        case 1: args::ret_int(info, md->mercenary.class_);       return;
        case 2: args::ret_str(info, md->db->name.c_str());        return;
        case 3: args::ret_int(info, mercenary_get_faith(md));     return;
        case 4: args::ret_int(info, mercenary_get_calls(md));     return;
        case 5: args::ret_int(info, md->mercenary.kill_count);    return;
        case 6: args::ret_int(info, static_cast<int>(mercenary_get_lifetime(md))); return;
        case 7: args::ret_int(info, md->db->lv);                  return;
        case 8: args::ret_int(info, md->bl.id);                   return;
        default: args::ret_null(info);
    }
}
} // namespace

void MercHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                 v8::Local<v8::Object> obj) {
    args::bind_method<MercHost>(iso, ctx, obj, this, "create",   &merc_create_cb);
    args::bind_method<MercHost>(iso, ctx, obj, this, "delete",   &merc_delete_cb);
    args::bind_method<MercHost>(iso, ctx, obj, this, "heal",     &merc_heal_cb);
    args::bind_method<MercHost>(iso, ctx, obj, this, "scStart",  &merc_scStart_cb);
    args::bind_method<MercHost>(iso, ctx, obj, this, "getCalls", &merc_getCalls_cb);
    args::bind_method<MercHost>(iso, ctx, obj, this, "setCalls", &merc_setCalls_cb);
    args::bind_method<MercHost>(iso, ctx, obj, this, "getFaith", &merc_getFaith_cb);
    args::bind_method<MercHost>(iso, ctx, obj, this, "setFaith", &merc_setFaith_cb);
    args::bind_method<MercHost>(iso, ctx, obj, this, "info",     &merc_info_cb);
    // Elemental info is part of a separate elemental.cpp subsystem,
    // not the mercenary one — keep as placeholder.
    bind_null(iso, ctx, obj, "elementalInfo");
}

// =====================================================================
// Party / Guild / Instance / Battleground / Channel
// =====================================================================

namespace {
void party_getName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int pid = args::int_arg(info, 0);
    auto* p = party_search(pid);
    args::ret_str(info, p ? p->party.name : "");
}
void party_getMembers_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int pid = args::int_arg(info, 0);
    auto* p = party_search(pid);
    auto out = v8::Array::New(args::iso(info));
    if (!p) { info.GetReturnValue().Set(out); return; }
    auto iso_ = args::iso(info); auto cx = args::ctx(info);
    uint32 idx_out = 0;
    for (int i = 0; i < MAX_PARTY; ++i) {
        if (p->party.member[i].account_id == 0) continue;
        auto row = v8::Object::New(iso_);
        (void)row->Set(cx, v8::String::NewFromUtf8(iso_, "name").ToLocalChecked(),
            v8::String::NewFromUtf8(iso_, p->party.member[i].name).ToLocalChecked());
        (void)row->Set(cx, v8::String::NewFromUtf8(iso_, "accountId").ToLocalChecked(),
            v8::Integer::NewFromUnsigned(iso_, p->party.member[i].account_id));
        (void)row->Set(cx, v8::String::NewFromUtf8(iso_, "charId").ToLocalChecked(),
            v8::Integer::NewFromUnsigned(iso_, p->party.member[i].char_id));
        (void)out->Set(cx, idx_out++, row);
    }
    info.GetReturnValue().Set(out);
}
void party_getLeader_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int pid = args::int_arg(info, 0);
    auto* p = party_search(pid);
    if (!p) { args::ret_int(info, 0); return; }
    for (int i = 0; i < MAX_PARTY; ++i) {
        if (p->party.member[i].leader) {
            args::ret_int(info, p->party.member[i].char_id);
            return;
        }
    }
    args::ret_int(info, 0);
}
void party_isLeader_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PartyHost>(info);
    if (!self || !self->sd() || self->sd()->status.party_id == 0) {
        args::ret_bool(info, false); return;
    }
    auto* p = party_search(self->sd()->status.party_id);
    if (!p) { args::ret_bool(info, false); return; }
    int char_id = self->sd()->status.char_id;
    for (int i = 0; i < MAX_PARTY; ++i) {
        if (p->party.member[i].char_id == static_cast<uint32>(char_id)) {
            args::ret_bool(info, p->party.member[i].leader != 0);
            return;
        }
    }
    args::ret_bool(info, false);
}
void party_create_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PartyHost>(info);
    if (!self || !self->sd()) { args::ret_int(info, -1); return; }
    auto name = args::str_arg(info, 0);
    char buf[NAME_LENGTH] = {};
    safestrncpy(buf, name.c_str(), NAME_LENGTH);
    int rc = party_create(*self->sd(), buf,
        args::bool_arg(info, 2) ? 1 : 0,
        args::int_arg(info, 3, 0));
    args::ret_int(info, rc);
}
void party_destroy_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int pid = args::int_arg(info, 0);
    auto* p = party_search(pid);
    if (p) party_broken(pid);
}
void party_delMember_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PartyHost>(info);
    if (!self || !self->sd()) return;
    party_removemember(*self->sd(), self->sd()->status.account_id,
                       self->sd()->status.name);
}
void party_changeLeader_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<PartyHost>(info);
    if (!self || !self->sd()) return;
    int char_id = args::int_arg(info, 1);
    auto* t_sd = map_charid2sd(char_id);
    if (!t_sd) return;
    auto* p = party_search(args::int_arg(info, 0));
    if (p) party_changeleader(self->sd(), t_sd, p);
}
} // namespace

void PartyHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                  v8::Local<v8::Object> obj) {
    args::bind_method<PartyHost>(iso, ctx, obj, this, "getName",     &party_getName_cb);
    args::bind_method<PartyHost>(iso, ctx, obj, this, "getMembers",  &party_getMembers_cb);
    args::bind_method<PartyHost>(iso, ctx, obj, this, "getLeader",   &party_getLeader_cb);
    args::bind_method<PartyHost>(iso, ctx, obj, this, "isLeader",    &party_isLeader_cb);
    args::bind_method<PartyHost>(iso, ctx, obj, this, "create",      &party_create_cb);
    args::bind_method<PartyHost>(iso, ctx, obj, this, "destroy",     &party_destroy_cb);
    args::bind_method<PartyHost>(iso, ctx, obj, this, "delMember",   &party_delMember_cb);
    args::bind_method<PartyHost>(iso, ctx, obj, this, "changeLeader",&party_changeLeader_cb);
    // Not yet wired — need party_invite resolution that requires the
    // target sd, plus party_changeoption details:
    bind_void(iso, ctx, obj, "addMember");
    bind_void(iso, ctx, obj, "changeOption");
}

namespace {
void guild_getName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = args::int_arg(info, 0);
    auto g = guild_search(gid);
    args::ret_str(info, g ? g->guild.name : "");
}
void guild_getMaster_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = args::int_arg(info, 0);
    auto g = guild_search(gid);
    args::ret_str(info, g ? g->guild.master : "");
}
void guild_getMasterId_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = args::int_arg(info, 0);
    auto g = guild_search(gid);
    args::ret_int(info, g && g->guild.member[0].char_id ? g->guild.member[0].char_id : 0);
}
void guild_info_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = args::int_arg(info, 0);
    int type = args::int_arg(info, 1);
    auto g = guild_search(gid);
    if (!g) { args::ret_int(info, 0); return; }
    int v = 0;
    switch (type) {
        case 0: v = g->guild.guild_lv; break;
        case 1: v = g->guild.max_member; break;
        case 2: v = g->guild.connect_member; break;
        case 3: v = g->guild.average_lv; break;
        case 4: v = static_cast<int>(g->guild.exp); break;
        case 5: v = static_cast<int>(g->guild.next_exp); break;
        case 6: v = g->guild.skill_point; break;
        default: break;
    }
    args::ret_int(info, v);
}
void guild_getMembers_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = args::int_arg(info, 0);
    auto g = guild_search(gid);
    auto out = v8::Array::New(args::iso(info));
    if (!g) { info.GetReturnValue().Set(out); return; }
    auto iso_ = args::iso(info); auto cx = args::ctx(info);
    uint32 idx_out = 0;
    for (int i = 0; i < MAX_GUILD; ++i) {
        if (g->guild.member[i].account_id == 0) continue;
        auto row = v8::Object::New(iso_);
        (void)row->Set(cx, v8::String::NewFromUtf8(iso_, "name").ToLocalChecked(),
            v8::String::NewFromUtf8(iso_, g->guild.member[i].name).ToLocalChecked());
        (void)row->Set(cx, v8::String::NewFromUtf8(iso_, "accountId").ToLocalChecked(),
            v8::Integer::NewFromUnsigned(iso_, g->guild.member[i].account_id));
        (void)row->Set(cx, v8::String::NewFromUtf8(iso_, "charId").ToLocalChecked(),
            v8::Integer::NewFromUnsigned(iso_, g->guild.member[i].char_id));
        (void)out->Set(cx, idx_out++, row);
    }
    info.GetReturnValue().Set(out);
}
void guild_getSkillLv_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int gid = args::int_arg(info, 0);
    int sid = args::int_arg(info, 1);
    auto g = guild_search(gid);
    if (!g) { args::ret_int(info, 0); return; }
    args::ret_int(info, guild_checkskill(g->guild, sid));
}
} // namespace

void GuildHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                  v8::Local<v8::Object> obj) {
    args::bind_method<GuildHost>(iso, ctx, obj, this, "getName",     &guild_getName_cb);
    args::bind_method<GuildHost>(iso, ctx, obj, this, "getMaster",   &guild_getMaster_cb);
    args::bind_method<GuildHost>(iso, ctx, obj, this, "getMasterId", &guild_getMasterId_cb);
    args::bind_method<GuildHost>(iso, ctx, obj, this, "info",        &guild_info_cb);
    args::bind_method<GuildHost>(iso, ctx, obj, this, "getMembers",  &guild_getMembers_cb);
    args::bind_method<GuildHost>(iso, ctx, obj, this, "getSkillLv",  &guild_getSkillLv_cb);
    // The rest depend on alliance / map-users / breakdown logic that
    // needs more rAthena helper exposure — keep as placeholders.
    bind_int0(iso, ctx, obj, "getAlliance");
    bind_int0(iso, ctx, obj, "getMapUsers");
    bind_void(iso, ctx, obj, "changeMaster");
    bind_void(iso, ctx, obj, "requestInfo");
}

namespace {
void instance_create_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<InstanceHost>(info);
    auto name = args::str_arg(info, 0);
    int mode = args::int_arg(info, 1, 0);
    int owner = args::int_arg(info, 2, self && self->sd() ? self->sd()->status.party_id : 0);
    int rc = instance_create(owner, name.c_str(), static_cast<e_instance_mode>(mode));
    args::ret_int(info, rc);
}
void instance_destroy_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int id = args::int_arg(info, 0);
    if (id > 0) instance_destroy(id);
}
void instance_enter_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<InstanceHost>(info);
    if (!self || !self->sd()) { args::ret_int(info, -1); return; }
    auto name = args::str_arg(info, 0);
    int x = args::int_arg(info, 1, -1), y = args::int_arg(info, 2, -1);
    int iid = args::int_arg(info, 4, 0);
    args::ret_int(info,
        instance_enter(self->sd(), iid, name.c_str(),
                       static_cast<int16>(x), static_cast<int16>(y)));
}
void instance_id_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<InstanceHost>(info);
    if (!self || !self->sd()) { args::ret_int(info, 0); return; }
    args::ret_int(info, self->sd()->instance_id);
}
} // namespace

namespace {
void instance_npcName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name = args::str_arg(info, 0);
    int instance_id = args::int_arg(info, 1, 0);
    auto* nd = npc_name2id(name.c_str());
    if (instance_id <= 0 || !nd) { args::ret_str(info, ""); return; }
    char buf[NAME_LENGTH];
    snprintf(buf, sizeof(buf), "dup_%d_%d", instance_id, nd->bl.id);
    args::ret_str(info, buf);
}

void instance_mapName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name = args::str_arg(info, 0);
    int instance_id = args::int_arg(info, 1, 0);
    int16 m;
    if (instance_id <= 0 ||
        (m = instance_mapid(map_mapname2mapid(name.c_str()), instance_id)) < 0) {
        args::ret_str(info, "");
        return;
    }
    args::ret_str(info, map_getmapdata(m)->name);
}

int32 instance_warpall_sub(struct block_list* bl, va_list ap) {
    uint32 m  = va_arg(ap, uint32);
    int32  x  = va_arg(ap, int32);
    int32  y  = va_arg(ap, int32);
    int32  id = va_arg(ap, int32);
    if (!bl || bl->type != BL_PC) return 0;
    auto* sd = reinterpret_cast<map_session_data*>(bl);
    auto idata = util::umap_find(instances, id);
    if (!idata) return 0;
    int owner = idata->owner_id;
    switch (idata->mode) {
        case IM_NONE: break;
        case IM_CHAR:  if ((int)sd->status.char_id  != owner) return 0; break;
        case IM_PARTY: if ((int)sd->status.party_id != owner) return 0; break;
        case IM_GUILD: if ((int)sd->status.guild_id != owner) return 0; break;
        case IM_CLAN:  if ((int)sd->status.clan_id  != owner) return 0; break;
        default: break;
    }
    pc_setpos(sd, m, x, y, CLR_TELEPORT);
    return 1;
}

void instance_warpAll_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto mapn = args::str_arg(info, 0);
    int x = args::int_arg(info, 1);
    int y = args::int_arg(info, 2);
    int id = args::int_arg(info, 3, 0);
    int16 m;
    if (id <= 0 || (m = map_mapname2mapid(mapn.c_str())) < 0 ||
        (m = instance_mapid(m, id)) < 0) return;
    auto idata = util::umap_find(instances, id);
    if (!idata) return;
    for (const auto& it : idata->map)
        map_foreachinmap(instance_warpall_sub, it.m, BL_PC,
                         map_id2index(m), x, y, id);
}

int32 instance_announce_sub(struct block_list* bl, va_list ap) {
    const char* mes = va_arg(ap, const char*);
    int32 type      = va_arg(ap, int32);
    if (!bl || bl->type != BL_PC) return 0;
    auto* sd = reinterpret_cast<map_session_data*>(bl);
    clif_broadcast(bl, mes, static_cast<int>(strlen(mes)+1), type, SELF);
    (void)sd;
    return 1;
}

void instance_announce_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int id = args::int_arg(info, 0, 0);
    auto mes = args::str_arg(info, 1);
    int flag = args::int_arg(info, 2, 0);
    auto idata = util::umap_find(instances, id);
    if (!idata) return;
    for (const auto& it : idata->map)
        map_foreachinmap(instance_announce_sub, it.m, BL_PC,
                         mes.c_str(), flag & BC_COLOR_MASK);
}

void instance_checkParty_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int party_id = args::int_arg(info, 0);
    int amount = args::int_arg(info, 1, 1);
    int min    = args::int_arg(info, 2, 1);
    int max    = args::int_arg(info, 3, MAX_LEVEL);
    if (amount < 1 || amount > MAX_PARTY ||
        min < 1 || min > MAX_LEVEL ||
        max < 1 || max > MAX_LEVEL) { args::ret_bool(info, false); return; }
    auto* p = party_search(party_id);
    if (!p) { args::ret_bool(info, false); return; }
    int count = 0;
    for (size_t i = 0; i < MAX_PARTY; i++) {
        auto* sd = p->data[i].sd;
        if (!sd || sd->state.autotrade) continue;
        if (sd->status.base_level < min || sd->status.base_level > max) {
            args::ret_bool(info, false); return;
        }
        ++count;
    }
    args::ret_bool(info, count >= amount);
}

void instance_checkGuild_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int guild_id = args::int_arg(info, 0);
    int amount = args::int_arg(info, 1, 1);
    int min    = args::int_arg(info, 2, 1);
    int max    = args::int_arg(info, 3, MAX_LEVEL);
    auto g = guild_search(guild_id);
    if (!g) { args::ret_bool(info, false); return; }
    int c = 0;
    for (int i = 0; i < MAX_GUILD; ++i) {
        auto* pl_sd = g->guild.member[i].sd;
        if (!pl_sd || !map_id2bl(pl_sd->bl.id) || pl_sd->state.autotrade) continue;
        if (pl_sd->status.base_level < min || pl_sd->status.base_level > max) {
            args::ret_bool(info, false); return;
        }
        ++c;
    }
    args::ret_bool(info, c >= amount);
}

void instance_info_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name = args::str_arg(info, 0);
    int type  = args::int_arg(info, 1);
    int index = args::int_arg(info, 2, 0);
    auto db = instance_search_db_name(name.c_str());
    if (!db) { args::ret_int(info, -1); return; }
    switch (type) {
        case IIT_ID:           args::ret_int(info, db->id); return;
        case IIT_TIME_LIMIT:   args::ret_int(info, static_cast<int>(db->limit)); return;
        case IIT_IDLE_TIMEOUT: args::ret_int(info, static_cast<int>(db->timeout)); return;
        case IIT_ENTER_MAP:    args::ret_str(info, map_mapid2mapname(db->enter.map)); return;
        case IIT_ENTER_X:      args::ret_int(info, db->enter.x); return;
        case IIT_ENTER_Y:      args::ret_int(info, db->enter.y); return;
        case IIT_MAPCOUNT:     args::ret_int(info, static_cast<int>(db->maplist.size())); return;
        case IIT_MAP:
            if (index < 0 || index >= (int)db->maplist.size()) {
                args::ret_str(info, ""); return;
            }
            args::ret_str(info, map_mapid2mapname(db->maplist[index]));
            return;
        default: args::ret_int(info, -1);
    }
}

void instance_liveInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int type = args::int_arg(info, 0);
    int id   = args::int_arg(info, 1, 0);
    if (type < ILI_NAME || type > ILI_OWNER) { args::ret_int(info, -1); return; }
    auto im = util::umap_find(instances, id);
    auto db = im ? instance_db.find(im->id) : nullptr;
    if (!im || !db) {
        if (type == ILI_NAME) args::ret_str(info, "");
        else                   args::ret_int(info, -1);
        return;
    }
    switch (type) {
        case ILI_NAME:  args::ret_str(info, db->name.c_str()); return;
        case ILI_MODE:  args::ret_int(info, im->mode);          return;
        case ILI_OWNER: args::ret_int(info, im->owner_id);      return;
    }
}

void instance_list_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto map_name = args::str_arg(info, 0);
    int mode = args::int_arg(info, 1, IM_MAX);
    int src_id = map_mapname2mapid(map_name.c_str());
    auto iso_ = args::iso(info); auto cx = args::ctx(info);
    auto out = v8::Array::New(iso_);
    if (src_id == 0) { info.GetReturnValue().Set(out); return; }
    uint32 idx_out = 0;
    for (int i = instance_start; i < map_num; ++i) {
        auto* md = &map[i];
        if (md->instance_src_map != src_id) continue;
        auto idata = util::umap_find(instances, md->instance_id);
        if (!idata) continue;
        if (mode != IM_MAX && idata->mode != mode) continue;
        (void)out->Set(cx, idx_out++,
            v8::Integer::New(iso_, md->instance_id));
    }
    info.GetReturnValue().Set(out);
}
} // namespace

void InstanceHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                     v8::Local<v8::Object> obj) {
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "create",     &instance_create_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "destroy",    &instance_destroy_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "enter",      &instance_enter_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "id",         &instance_id_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "npcName",    &instance_npcName_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "mapName",    &instance_mapName_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "warpAll",    &instance_warpAll_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "announce",   &instance_announce_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "checkParty", &instance_checkParty_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "checkGuild", &instance_checkGuild_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "info",       &instance_info_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "liveInfo",   &instance_liveInfo_cb);
    args::bind_method<InstanceHost>(iso, ctx, obj, this, "list",       &instance_list_cb);
    // checkClan needs clan.hpp clan_search exposure path that's
    // platform-quirky; getVar/setVar would need scope_t reg helpers
    // exposed — placeholder for now.
    bind_false(iso, ctx, obj, "checkClan");
    bind_null (iso, ctx, obj, "getVar");
    bind_void (iso, ctx, obj, "setVar");
}

namespace {
void bg_create_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name = args::str_arg(info, 0);
    int x = args::int_arg(info, 1);
    int y = args::int_arg(info, 2);
    int mapindex = 0;
    if (name != "-" && (mapindex = mapindex_name2id(name.c_str())) == 0) {
        args::ret_int(info, 0); return;
    }
    s_battleground_team team;
    team.warp_x = x;
    team.warp_y = y;
    team.quit_event  = args::str_arg(info, 3);
    team.death_event = args::str_arg(info, 4);
    args::ret_int(info, bg_create(static_cast<uint16>(mapindex), &team));
}

void bg_join_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<BattlegroundHost>(info);
    int bg_id = args::int_arg(info, 0);
    auto bg = util::umap_find(bg_team_db, bg_id);
    if (!bg) { args::ret_int(info, 0); return; }
    int mapindex, x, y;
    if (info.Length() > 1 && info[1]->IsString()) {
        auto map_name = args::str_arg(info, 1);
        mapindex = mapindex_name2id(map_name.c_str());
        if (!mapindex) { args::ret_int(info, 0); return; }
        x = args::int_arg(info, 2);
        y = args::int_arg(info, 3);
    } else {
        mapindex = bg->cemetery.map;
        x = bg->cemetery.x;
        y = bg->cemetery.y;
    }
    int char_id = args::int_arg(info, 4, 0);
    map_session_data* sd = char_id ? map_charid2sd(char_id) : self ? self->sd() : nullptr;
    if (!sd) { args::ret_int(info, 0); return; }
    if (!map_getmapflag(map_mapindex2mapid(mapindex), MF_BATTLEGROUND)) {
        args::ret_int(info, 0); return;
    }
    bool ok = bg_team_join(bg_id, sd, false) &&
              pc_setpos(sd, mapindex, x, y, CLR_TELEPORT) == SETPOS_OK;
    args::ret_int(info, ok ? 1 : 0);
}

void bg_setTeamXY_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int bg_id = args::int_arg(info, 0);
    auto bg = util::umap_find(bg_team_db, bg_id);
    if (!bg) return;
    bg->cemetery.x = args::int_arg(info, 1);
    bg->cemetery.y = args::int_arg(info, 2);
}

void bg_reserve_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name  = args::str_arg(info, 0);
    bool ended = args::bool_arg(info, 1);
    args::ret_int(info, bg_queue_reserve(name.c_str(), ended) ? 1 : 0);
}

void bg_unbook_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name = args::str_arg(info, 0);
    bg_queue_unbook(name.c_str());
}

void bg_desert_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<BattlegroundHost>(info);
    int char_id = args::int_arg(info, 0, 0);
    map_session_data* sd = char_id ? map_charid2sd(char_id) : self ? self->sd() : nullptr;
    if (!sd || !sd->bg_id) return;
    bg_team_leave(sd, false, true);
}

void bg_warp_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int bg_id = args::int_arg(info, 0);
    auto name = args::str_arg(info, 1);
    int x = args::int_arg(info, 2);
    int y = args::int_arg(info, 3);
    int mapindex = mapindex_name2id(name.c_str());
    if (!mapindex) return;
    bg_team_warp(bg_id, static_cast<uint16>(mapindex),
                 static_cast<int16>(x), static_cast<int16>(y));
}

void bg_spawnMonster_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int bg_id = args::int_arg(info, 0);
    auto mapn = args::str_arg(info, 1);
    int x  = args::int_arg(info, 2);
    int y  = args::int_arg(info, 3);
    auto disp = args::str_arg(info, 4);
    int cls = args::int_arg(info, 5);
    auto evt = args::str_arg(info, 6);
    args::ret_int(info, mob_spawn_bg(mapn.c_str(),
        static_cast<int16>(x), static_cast<int16>(y),
        disp.c_str(), cls, evt.c_str(),
        static_cast<uint32>(bg_id)));
}

void bg_setMonsterTeam_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int id    = args::int_arg(info, 0);
    int bg_id = args::int_arg(info, 1);
    auto* mbl = map_id2bl(id);
    if (!mbl || mbl->type != BL_MOB) return;
    auto* md = reinterpret_cast<mob_data*>(mbl);
    md->bg_id = static_cast<uint32>(bg_id);
    unit_stop_attack(&md->bl);
    unit_stop_walking(&md->bl, USW_NONE);
    md->target_id = md->attacked_id = 0;
    clif_name_area(&md->bl);
}

void bg_leave_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<BattlegroundHost>(info);
    int char_id = args::int_arg(info, 0, 0);
    map_session_data* sd = char_id ? map_charid2sd(char_id) : self ? self->sd() : nullptr;
    if (!sd || !sd->bg_id) return;
    bg_team_leave(sd, false, false);
}

void bg_destroy_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int bg_id = args::int_arg(info, 0);
    if (bg_id > 0) bg_team_delete(bg_id);
}

void bg_areaUsers_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int bg_id = args::int_arg(info, 0);
    auto map_name = args::str_arg(info, 1);
    int x0 = args::int_arg(info, 2), y0 = args::int_arg(info, 3);
    int x1 = args::int_arg(info, 4), y1 = args::int_arg(info, 5);
    int m = map_mapname2mapid(map_name.c_str());
    auto bg = util::umap_find(bg_team_db, bg_id);
    if (!bg || m < 0) { args::ret_int(info, 0); return; }
    int c = 0;
    for (const auto& member : bg->members) {
        if (member.sd->bl.m != m) continue;
        if (member.sd->bl.x < x0 || member.sd->bl.y < y0 ||
            member.sd->bl.x > x1 || member.sd->bl.y > y1) continue;
        ++c;
    }
    args::ret_int(info, c);
}

void bg_updateScore_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto map_name = args::str_arg(info, 0);
    int m = map_mapname2mapid(map_name.c_str());
    if (m < 0) return;
    auto* mapdata = map_getmapdata(m);
    mapdata->bgscore_lion  = args::int_arg(info, 1);
    mapdata->bgscore_eagle = args::int_arg(info, 2);
    clif_bg_updatescore(m);
}

void bg_getData_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    int bg_id = args::int_arg(info, 0);
    int type  = args::int_arg(info, 1);
    auto bg = util::umap_find(bg_team_db, bg_id);
    if (!bg) { args::ret_int(info, 0); return; }
    switch (type) {
        case 0: args::ret_int(info, static_cast<int>(bg->members.size())); return;
        case 1: args::ret_int(info, static_cast<int>(bg->members.size())); return;
        default: args::ret_int(info, 0);
    }
}

void bg_info_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name = args::str_arg(info, 0);
    int type  = args::int_arg(info, 1);
    auto bg = bg_search_name(name.c_str());
    if (!bg) { args::ret_null(info); return; }
    switch (type) {
        case BG_INFO_ID:               args::ret_int(info, bg->id); return;
        case BG_INFO_REQUIRED_PLAYERS: args::ret_int(info, bg->required_players); return;
        case BG_INFO_MAX_PLAYERS:      args::ret_int(info, bg->max_players);      return;
        case BG_INFO_MIN_LEVEL:        args::ret_int(info, bg->min_lvl);          return;
        case BG_INFO_MAX_LEVEL:        args::ret_int(info, bg->max_lvl);          return;
        case BG_INFO_DESERTER_TIME:    args::ret_int(info, bg->deserter_time);    return;
        case BG_INFO_MAPS:             args::ret_int(info, static_cast<int>(bg->maps.size())); return;
        default: args::ret_null(info);
    }
}
} // namespace

void BattlegroundHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                         v8::Local<v8::Object> obj) {
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "create",         &bg_create_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "join",           &bg_join_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "setTeamXY",      &bg_setTeamXY_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "reserve",        &bg_reserve_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "unbook",         &bg_unbook_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "desert",         &bg_desert_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "warp",           &bg_warp_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "spawnMonster",   &bg_spawnMonster_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "setMonsterTeam", &bg_setMonsterTeam_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "leave",          &bg_leave_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "destroy",        &bg_destroy_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "areaUsers",      &bg_areaUsers_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "updateScore",    &bg_updateScore_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "getData",        &bg_getData_cb);
    args::bind_method<BattlegroundHost>(iso, ctx, obj, this, "info",           &bg_info_cb);
    // waitingRoomToBg / waitingRoomToBgSingle need chat_data / npc context
    // wiring that lives in npc_chat — placeholder for now.
    bind_void(iso, ctx, obj, "waitingRoomToBgSingle");
    bind_void(iso, ctx, obj, "waitingRoomToBg");
}

namespace {
void channel_create_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto name = args::str_arg(info, 0);
    auto pass = args::str_arg(info, 2);
    char nbuf[64] = {}, pbuf[64] = {};
    safestrncpy(nbuf, name.c_str(), sizeof(nbuf));
    safestrncpy(pbuf, pass.c_str(), sizeof(pbuf));
    channel_create_simple(nbuf, pbuf, CHAN_TYPE_PUBLIC, 0);
}
void channel_join_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<ChannelHost>(info);
    if (!self || !self->sd()) return;
    auto name = args::str_arg(info, 0);
    auto* c = channel_name2channel(const_cast<char*>(name.c_str()), self->sd(), 0);
    if (c) channel_join(c, self->sd());
}
void channel_chat_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* self = args::unwrap<ChannelHost>(info);
    if (!self || !self->sd()) return;
    auto name = args::str_arg(info, 0);
    auto msg  = args::str_arg(info, 1);
    auto* c = channel_name2channel(const_cast<char*>(name.c_str()), self->sd(), 0);
    if (c) channel_send(c, self->sd(), msg.c_str());
}
} // namespace

void ChannelHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                    v8::Local<v8::Object> obj) {
    args::bind_method<ChannelHost>(iso, ctx, obj, this, "create", &channel_create_cb);
    args::bind_method<ChannelHost>(iso, ctx, obj, this, "join",   &channel_join_cb);
    args::bind_method<ChannelHost>(iso, ctx, obj, this, "chat",   &channel_chat_cb);
    // Setters / kick / ban / unban / setGroups need channel field
    // mutators that aren't surfaced — placeholder for now.
    bind_void(iso, ctx, obj, "setOption");
    bind_int0(iso, ctx, obj, "getOption");
    bind_void(iso, ctx, obj, "setColor");
    bind_void(iso, ctx, obj, "setPassword");
    bind_void(iso, ctx, obj, "setGroups");
    bind_void(iso, ctx, obj, "ban");
    bind_void(iso, ctx, obj, "unban");
    bind_void(iso, ctx, obj, "kick");
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
