// Aggregated host classes for the remaining sub-objects:
//   - NpcInfoHost      (ctx.npc)
//   - QuestHost / AchievementHost / StorageHost / CartHost /
//     MailHost / PetHost / HomHost / MercHost  (ctx.player.<sub>)
//   - PartyHost / GuildHost / InstanceHost /
//     BattlegroundHost / ChannelHost  (ctx.<sub>)
//
// Every TS method on these interfaces has a real C++ callback. Most are
// type-correct placeholders today; real bodies are added incrementally
// as the underlying rAthena subsystems prove out. No method falls back
// to the generic `stub_callback` — once a host installs onto a JS
// object, it owns every name on it.

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <v8.h>

struct map_session_data;
struct npc_data;

namespace rathena::scripting {

// Common shape for every host class below — single map_session_data
// reference (or npc_data for NpcInfoHost) + install_on_object.
class NpcInfoHost {
public:
    explicit NpcInfoHost(npc_data& nd) : nd_(nd) {}
    void install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                           v8::Local<v8::Object> obj);
    npc_data& nd() { return nd_; }
private:
    npc_data& nd_;
};

#define DECLARE_PLAYER_SUBHOST(Cls) \
    class Cls { \
    public: \
        explicit Cls(map_session_data& sd) : sd_(sd) {} \
        void install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx, \
                               v8::Local<v8::Object> obj); \
        map_session_data& sd() { return sd_; } \
    private: \
        map_session_data& sd_; \
    }

DECLARE_PLAYER_SUBHOST(QuestHost);
DECLARE_PLAYER_SUBHOST(AchievementHost);
DECLARE_PLAYER_SUBHOST(StorageHost);
DECLARE_PLAYER_SUBHOST(CartHost);
DECLARE_PLAYER_SUBHOST(MailHost);
DECLARE_PLAYER_SUBHOST(PetHost);
DECLARE_PLAYER_SUBHOST(HomHost);
DECLARE_PLAYER_SUBHOST(MercHost);

#undef DECLARE_PLAYER_SUBHOST

// Social hosts take an optional attached map_session_data (the caller)
// — most ops take explicit ids (partyId, guildId, …) but a few default
// to the caller's affiliation.
#define DECLARE_SOCIAL_HOST(Cls) \
    class Cls { \
    public: \
        explicit Cls(map_session_data* attached) : sd_(attached) {} \
        void install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx, \
                               v8::Local<v8::Object> obj); \
        map_session_data* sd() { return sd_; } \
    private: \
        map_session_data* sd_; \
    }

DECLARE_SOCIAL_HOST(PartyHost);
DECLARE_SOCIAL_HOST(GuildHost);
DECLARE_SOCIAL_HOST(InstanceHost);
DECLARE_SOCIAL_HOST(BattlegroundHost);
DECLARE_SOCIAL_HOST(ChannelHost);

#undef DECLARE_SOCIAL_HOST

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
