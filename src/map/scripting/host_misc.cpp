#ifdef HAVE_TS_SCRIPTING

#include "host_misc.hpp"

#include "arg_helpers.hpp"
#include "../achievement.hpp"
#include "../channel.hpp"
#include "../clif.hpp"
#include "../guild.hpp"
#include "../instance.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../party.hpp"
#include "../pc.hpp"
#include "../quest.hpp"
#include "../status.hpp"
#include "../storage.hpp"

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

void NpcInfoHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                    v8::Local<v8::Object> obj) {
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "hide",    &npc_hide_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "show",    &npc_show_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "disable", &npc_disable_cb);
    args::bind_method<NpcInfoHost>(iso, ctx, obj, this, "enable",  &npc_enable_cb);

    // Display / movement
    bind_void(iso, ctx, obj, "setDisplay");
    bind_void(iso, ctx, obj, "speed");
    bind_void(iso, ctx, obj, "walkTo");
    bind_void(iso, ctx, obj, "stop");
    bind_void(iso, ctx, obj, "moveTo");
    bind_void(iso, ctx, obj, "duplicateDynamic");

    // Shop
    bind_void(iso, ctx, obj, "shopSet");
    bind_void(iso, ctx, obj, "shopAdd");
    bind_void(iso, ctx, obj, "shopDel");
    bind_void(iso, ctx, obj, "shopAttach");
    bind_void(iso, ctx, obj, "shopUpdate");

    // Waiting room
    bind_void(iso, ctx, obj, "createWaitingRoom");
    bind_void(iso, ctx, obj, "removeWaitingRoom");
    bind_void(iso, ctx, obj, "enableWaitingRoom");
    bind_void(iso, ctx, obj, "disableWaitingRoom");
    bind_int0(iso, ctx, obj, "getWaitingRoomState");
    bind_void(iso, ctx, obj, "warpWaitingPc");
    bind_void(iso, ctx, obj, "kickWaitingRoomUser");
    bind_void(iso, ctx, obj, "kickAllWaitingRoom");
    bind_int0(iso, ctx, obj, "getWaitingRoomUsers");

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
} // namespace

void QuestHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                  v8::Local<v8::Object> obj) {
    args::bind_method<QuestHost>(iso, ctx, obj, this, "add",      &quest_add_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "complete", &quest_complete_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "erase",    &quest_erase_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "change",   &quest_change_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "check",    &quest_check_cb);
    args::bind_method<QuestHost>(iso, ctx, obj, this, "isBegin",  &quest_isBegin_cb);
    // Quest UI hooks — clif_quest_show_event etc. — need event-marker
    // helpers we haven't surfaced yet.
    bind_void(iso, ctx, obj, "showEvent");
    bind_void(iso, ctx, obj, "refreshInfo");
    bind_void(iso, ctx, obj, "showInfo");
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
} // namespace

void AchievementHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                        v8::Local<v8::Object> obj) {
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "add",      &ach_add_cb);
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "remove",   &ach_remove_cb);
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "exists",   &ach_exists_cb);
    args::bind_method<AchievementHost>(iso, ctx, obj, this, "info",     &ach_info_cb);
    // achievement_update needs the full update_data shape; complete uses
    // a check-then-finish path. Both deferred.
    bind_void(iso, ctx, obj, "complete");
    bind_void(iso, ctx, obj, "update");
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

void MailHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                 v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_void(iso, ctx, obj, "open");
}

void PetHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_void(iso, ctx, obj, "catchPet");
    bind_void(iso, ctx, obj, "makePet");
    bind_void(iso, ctx, obj, "birthPet");
    bind_void(iso, ctx, obj, "openIncubator");
    bind_null(iso, ctx, obj, "info");
    bind_void(iso, ctx, obj, "skillBonus");
    bind_void(iso, ctx, obj, "skillSupport");
    bind_void(iso, ctx, obj, "skillAttack");
    bind_void(iso, ctx, obj, "skillAttack2");
    bind_void(iso, ctx, obj, "recovery");
    bind_void(iso, ctx, obj, "loot");
}

void HomHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_false(iso, ctx, obj, "exists");
    bind_false(iso, ctx, obj, "isCalled");
    bind_null (iso, ctx, obj, "info");
    bind_void (iso, ctx, obj, "evolve");
    bind_void (iso, ctx, obj, "morph");
    bind_void (iso, ctx, obj, "mutate");
    bind_void (iso, ctx, obj, "shuffle");
    bind_void (iso, ctx, obj, "addIntimacy");
}

void MercHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                 v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_void(iso, ctx, obj, "create");
    bind_void(iso, ctx, obj, "delete");
    bind_void(iso, ctx, obj, "heal");
    bind_void(iso, ctx, obj, "scStart");
    bind_int0(iso, ctx, obj, "getCalls");
    bind_void(iso, ctx, obj, "setCalls");
    bind_int0(iso, ctx, obj, "getFaith");
    bind_void(iso, ctx, obj, "setFaith");
    bind_null(iso, ctx, obj, "info");
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

void InstanceHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                     v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_int0(iso, ctx, obj, "create");
    bind_void(iso, ctx, obj, "destroy");
    bind_int0(iso, ctx, obj, "enter");
    bind_str (iso, ctx, obj, "npcName");
    bind_str (iso, ctx, obj, "mapName");
    bind_int0(iso, ctx, obj, "id");
    bind_void(iso, ctx, obj, "warpAll");
    bind_void(iso, ctx, obj, "announce");
    bind_false(iso, ctx, obj, "checkParty");
    bind_false(iso, ctx, obj, "checkGuild");
    bind_false(iso, ctx, obj, "checkClan");
    bind_null(iso, ctx, obj, "info");
    bind_null(iso, ctx, obj, "liveInfo");
    bind_arr (iso, ctx, obj, "list");
    bind_null(iso, ctx, obj, "getVar");
    bind_void(iso, ctx, obj, "setVar");
}

void BattlegroundHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                         v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_int0(iso, ctx, obj, "create");
    bind_int0(iso, ctx, obj, "join");
    bind_void(iso, ctx, obj, "setTeamXY");
    bind_int0(iso, ctx, obj, "reserve");
    bind_void(iso, ctx, obj, "unbook");
    bind_void(iso, ctx, obj, "desert");
    bind_void(iso, ctx, obj, "warp");
    bind_int0(iso, ctx, obj, "spawnMonster");
    bind_void(iso, ctx, obj, "setMonsterTeam");
    bind_void(iso, ctx, obj, "leave");
    bind_void(iso, ctx, obj, "destroy");
    bind_void(iso, ctx, obj, "waitingRoomToBgSingle");
    bind_void(iso, ctx, obj, "waitingRoomToBg");
    bind_int0(iso, ctx, obj, "getData");
    bind_int0(iso, ctx, obj, "areaUsers");
    bind_void(iso, ctx, obj, "updateScore");
    bind_null(iso, ctx, obj, "info");
}

void ChannelHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                    v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_void(iso, ctx, obj, "create");
    bind_void(iso, ctx, obj, "join");
    bind_void(iso, ctx, obj, "setOption");
    bind_int0(iso, ctx, obj, "getOption");
    bind_void(iso, ctx, obj, "setColor");
    bind_void(iso, ctx, obj, "setPassword");
    bind_void(iso, ctx, obj, "setGroups");
    bind_void(iso, ctx, obj, "chat");
    bind_void(iso, ctx, obj, "ban");
    bind_void(iso, ctx, obj, "unban");
    bind_void(iso, ctx, obj, "kick");
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
