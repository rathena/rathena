// WorldHost — backs every method on ctx.world. World-level ops (no
// per-player context required by definition, though many take a
// player as an implicit attached rid).

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <v8.h>

struct map_session_data;

namespace rathena::scripting {

class WorldHost {
public:
    explicit WorldHost(map_session_data* attached) : sd_(attached) {}

    void install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                           v8::Local<v8::Object> obj);

    map_session_data* sd() { return sd_; }

private:
    // The "attached rid" — used by ops that broadcast from the
    // caller's position or look at the caller's map. May be null
    // (onInit / onTimer hooks).
    map_session_data* sd_;

#define WH_CB(name) \
    static void name##_cb(const v8::FunctionCallbackInfo<v8::Value>& info)

    WH_CB(now);
    WH_CB(announce); WH_CB(mapAnnounce); WH_CB(areaAnnounce);
    WH_CB(globalMessage); WH_CB(debugMessage); WH_CB(errorMessage); WH_CB(logMessage);
    WH_CB(soundEffectAll); WH_CB(playBgmAll);
    WH_CB(spawnMob); WH_CB(spawnAreaMob); WH_CB(spawnGuardian);
    WH_CB(guardianInfo);
    WH_CB(killMonster); WH_CB(killMonsterAll); WH_CB(mobCount);
    WH_CB(respawnGuildOwned); WH_CB(getRandomMobId);
    WH_CB(getMonsterInfo); WH_CB(getMobDrops); WH_CB(mobInfo);
    WH_CB(unitWalk); WH_CB(unitWalkToTarget); WH_CB(unitAttack);
    WH_CB(unitKill); WH_CB(unitWarp); WH_CB(unitTalk);
    WH_CB(unitSkillUseId); WH_CB(unitSkillUsePos);
    WH_CB(unitStopAttack); WH_CB(unitStopWalk);
    WH_CB(unitExists); WH_CB(getUnitType); WH_CB(getUnitName);
    WH_CB(setUnitName); WH_CB(getUnitTitle); WH_CB(setUnitTitle);
    WH_CB(getUnitData); WH_CB(setUnitData);
    WH_CB(getUnits); WH_CB(getMapUnits); WH_CB(getAreaUnits);
    WH_CB(getMapUsers); WH_CB(getAreaUsers); WH_CB(getServerUsers);
    WH_CB(isLoggedIn); WH_CB(ridToName); WH_CB(getAreaDropItem);
    WH_CB(mapIdToName); WH_CB(getMapXY); WH_CB(distance);
    WH_CB(setCell); WH_CB(checkCell); WH_CB(getFreeCell);
    WH_CB(setWall); WH_CB(delWall); WH_CB(checkWall);
    WH_CB(makeItem); WH_CB(cleanArea); WH_CB(cleanMap);
    WH_CB(warpPortal); WH_CB(mapWarp); WH_CB(areaWarp);
    WH_CB(warpParty); WH_CB(warpGuild); WH_CB(areaPercentHeal);
    WH_CB(attachRid); WH_CB(addRid); WH_CB(playerAttached); WH_CB(getAttachedRid);
    WH_CB(setMapFlag); WH_CB(removeMapFlag); WH_CB(getMapFlag); WH_CB(setMapFlagNoSave);
    WH_CB(day); WH_CB(night); WH_CB(isDay); WH_CB(isNight);
    WH_CB(pvpOn); WH_CB(pvpOff); WH_CB(gvgOn); WH_CB(gvgOff);
    WH_CB(gvgOn3); WH_CB(gvgOff3);
    WH_CB(agitStart); WH_CB(agitEnd); WH_CB(agitCheck);
    WH_CB(flagEmblem); WH_CB(castleName); WH_CB(castleData);
    WH_CB(getTime); WH_CB(getTimeTick); WH_CB(getTimeStr);
    WH_CB(setBattleFlag); WH_CB(getBattleFlag);
    WH_CB(atCommand); WH_CB(charCommand); WH_CB(useAtCommand);
    WH_CB(bindAtCommand); WH_CB(unbindAtCommand);
    WH_CB(itemName); WH_CB(itemSlots); WH_CB(itemInfo);
    WH_CB(setItemInfo); WH_CB(setItemScript);
    WH_CB(gmLevel); WH_CB(groupId); WH_CB(itemLink);

#undef WH_CB
};

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
