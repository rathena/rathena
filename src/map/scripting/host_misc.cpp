#ifdef HAVE_TS_SCRIPTING

#include "host_misc.hpp"

#include "arg_helpers.hpp"
#include "../clif.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../pc.hpp"

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

void QuestHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                  v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_void(iso, ctx, obj, "add");
    bind_void(iso, ctx, obj, "complete");
    bind_void(iso, ctx, obj, "erase");
    bind_void(iso, ctx, obj, "change");
    bind_int0(iso, ctx, obj, "check");
    bind_false(iso, ctx, obj, "isBegin");
    bind_void(iso, ctx, obj, "showEvent");
    bind_void(iso, ctx, obj, "refreshInfo");
    bind_void(iso, ctx, obj, "showInfo");
}

void AchievementHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                        v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_void(iso, ctx, obj, "add");
    bind_void(iso, ctx, obj, "remove");
    bind_void(iso, ctx, obj, "complete");
    bind_false(iso, ctx, obj, "exists");
    bind_int0(iso, ctx, obj, "info");
    bind_void(iso, ctx, obj, "update");
}

void StorageHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                    v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_void(iso, ctx, obj, "open");
    bind_void(iso, ctx, obj, "openExtra");
    bind_int0(iso, ctx, obj, "countItem");
    bind_void(iso, ctx, obj, "delItem");
    bind_void(iso, ctx, obj, "openGuildStorage");
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

void PartyHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                  v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_str (iso, ctx, obj, "getName");
    bind_arr (iso, ctx, obj, "getMembers");
    bind_int0(iso, ctx, obj, "getLeader");
    bind_false(iso, ctx, obj, "isLeader");
    bind_int0(iso, ctx, obj, "create");
    bind_void(iso, ctx, obj, "destroy");
    bind_void(iso, ctx, obj, "addMember");
    bind_void(iso, ctx, obj, "delMember");
    bind_void(iso, ctx, obj, "changeLeader");
    bind_void(iso, ctx, obj, "changeOption");
}

void GuildHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                  v8::Local<v8::Object> obj) {
    (void)sd_;
    bind_str (iso, ctx, obj, "getName");
    bind_str (iso, ctx, obj, "getMaster");
    bind_int0(iso, ctx, obj, "getMasterId");
    bind_int0(iso, ctx, obj, "info");
    bind_arr (iso, ctx, obj, "getMembers");
    bind_int0(iso, ctx, obj, "getSkillLv");
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
