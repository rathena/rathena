// PlayerHost — backs every method on ctx.player. One instance per
// active DialogContext; lifetime tied to the dialog session.
//
// Each TS method (heal, warp, giveItem, …) has a matching static
// callback on this class. install_on_object() binds them onto the
// live ctx.player JS object, overriding any auto-generated stubs of
// the same name.

#pragma once

#ifdef HAVE_TS_SCRIPTING

#include <memory>
#include <v8.h>
#include <vector>

struct map_session_data;

namespace rathena::scripting {

class PlayerHost;

// One slot per var-bag proxy on ctx.player (perm / session / account /
// accountGlobal). The External pointer in each property-handler
// callback dereferences into this; the host owns the slots so they
// outlive every JS access for the dialog's lifetime.
struct VarProxySlot {
    PlayerHost* host;
    const char* prefix;
};

class PlayerHost {
public:
    explicit PlayerHost(map_session_data& sd) : sd_(sd) {}

    // Bind every method on the supplied JS object (typically the
    // ctx.player sub-object created by V8 from the surface_stubs
    // ObjectTemplate tree). Called once per click from DialogContext::to_js.
    void install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                           v8::Local<v8::Object> obj);

    // Install `ctx.player.{perm,session,account,accountGlobal}` — JS
    // proxies that read/write rAthena's reg scopes via pc_setreg2 /
    // pc_readreg2. Called from install_on_object after the regular
    // method bindings.
    void install_perm_proxy(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                            v8::Local<v8::Object> obj);

    map_session_data& sd() { return sd_; }

private:
    map_session_data& sd_;
    // One slot per var-bag proxy (perm/session/account/accountGlobal).
    // External pointers in V8 callbacks dereference into these — they
    // must outlive every JS access, so the host owns them.
    std::vector<std::unique_ptr<VarProxySlot>> slots_;

    // V8 callbacks. Each is a static thunk that unwraps `this` from
    // FunctionCallbackInfo::Data and forwards to a member function.
    // Declared via macros below to keep boilerplate minimal.
#define PLAYER_HOST_CALLBACK(name) \
    static void name##_cb(const v8::FunctionCallbackInfo<v8::Value>& info)

    // Heal / SP / AP
    PLAYER_HOST_CALLBACK(heal);
    PLAYER_HOST_CALLBACK(healAp);
    PLAYER_HOST_CALLBACK(itemHeal);
    PLAYER_HOST_CALLBACK(percentHeal);
    PLAYER_HOST_CALLBACK(recovery);

    // Experience / level
    PLAYER_HOST_CALLBACK(giveExp);
    PLAYER_HOST_CALLBACK(baseExpRatio);
    PLAYER_HOST_CALLBACK(jobExpRatio);

    // Job / class
    PLAYER_HOST_CALLBACK(jobChange);
    PLAYER_HOST_CALLBACK(changeBase);
    PLAYER_HOST_CALLBACK(changeSex);
    PLAYER_HOST_CALLBACK(jobName);

    // Movement
    PLAYER_HOST_CALLBACK(warp);
    PLAYER_HOST_CALLBACK(savePoint);
    PLAYER_HOST_CALLBACK(save);
    PLAYER_HOST_CALLBACK(getSavePoint);
    PLAYER_HOST_CALLBACK(pushPc);
    PLAYER_HOST_CALLBACK(warpPartner);

    // Items
    PLAYER_HOST_CALLBACK(giveItem);
    PLAYER_HOST_CALLBACK(giveRentItem);
    PLAYER_HOST_CALLBACK(giveNamedItem);
    PLAYER_HOST_CALLBACK(giveRandomGroupItem);
    PLAYER_HOST_CALLBACK(giveGroupItem);
    PLAYER_HOST_CALLBACK(delItem);
    PLAYER_HOST_CALLBACK(delItemAtIndex);
    PLAYER_HOST_CALLBACK(countItem);
    PLAYER_HOST_CALLBACK(countBound);
    PLAYER_HOST_CALLBACK(hasItem);
    PLAYER_HOST_CALLBACK(clearItems);
    PLAYER_HOST_CALLBACK(consumeItem);
    PLAYER_HOST_CALLBACK(searchItem);
    PLAYER_HOST_CALLBACK(getInventory);
    PLAYER_HOST_CALLBACK(mergeItems);
    PLAYER_HOST_CALLBACK(identifyAll);
    PLAYER_HOST_CALLBACK(checkWeight);

    // Equipment
    PLAYER_HOST_CALLBACK(getEquipId);
    PLAYER_HOST_CALLBACK(getEquipName);
    PLAYER_HOST_CALLBACK(getEquipUniqueId);
    PLAYER_HOST_CALLBACK(getEquipRefine);
    PLAYER_HOST_CALLBACK(getEquipWeaponLv);
    PLAYER_HOST_CALLBACK(getEquipArmorLv);
    PLAYER_HOST_CALLBACK(getEquipCardCount);
    PLAYER_HOST_CALLBACK(getEquipCardId);
    PLAYER_HOST_CALLBACK(getEnchantGrade);
    PLAYER_HOST_CALLBACK(isEquipped);
    PLAYER_HOST_CALLBACK(isEquipEnableRef);
    PLAYER_HOST_CALLBACK(getItemPos);
    PLAYER_HOST_CALLBACK(equip);
    PLAYER_HOST_CALLBACK(autoEquip);
    PLAYER_HOST_CALLBACK(unequip);
    PLAYER_HOST_CALLBACK(delEquip);
    PLAYER_HOST_CALLBACK(breakEquip);
    PLAYER_HOST_CALLBACK(successRefine);
    PLAYER_HOST_CALLBACK(failRefine);
    PLAYER_HOST_CALLBACK(downRefine);
    PLAYER_HOST_CALLBACK(repair);
    PLAYER_HOST_CALLBACK(repairAll);
    PLAYER_HOST_CALLBACK(removeCards);
    PLAYER_HOST_CALLBACK(getBrokenId);

    // Skills
    PLAYER_HOST_CALLBACK(skillLv);
    PLAYER_HOST_CALLBACK(addSkill);
    PLAYER_HOST_CALLBACK(itemSkill);
    PLAYER_HOST_CALLBACK(getSkillList);
    PLAYER_HOST_CALLBACK(skillPointCount);
    PLAYER_HOST_CALLBACK(basicSkillCheck);

    // Looks / mounts
    PLAYER_HOST_CALLBACK(setLook);
    PLAYER_HOST_CALLBACK(changeLook);
    PLAYER_HOST_CALLBACK(getLook);
    PLAYER_HOST_CALLBACK(setFont);
    PLAYER_HOST_CALLBACK(setCart);
    PLAYER_HOST_CALLBACK(setFalcon);
    PLAYER_HOST_CALLBACK(setRiding);
    PLAYER_HOST_CALLBACK(setDragon);
    PLAYER_HOST_CALLBACK(setMadogear);
    PLAYER_HOST_CALLBACK(setMounting);
    PLAYER_HOST_CALLBACK(checkCart);
    PLAYER_HOST_CALLBACK(checkFalcon);
    PLAYER_HOST_CALLBACK(checkRiding);
    PLAYER_HOST_CALLBACK(checkDragon);
    PLAYER_HOST_CALLBACK(checkMadogear);
    PLAYER_HOST_CALLBACK(checkWug);
    PLAYER_HOST_CALLBACK(isMounting);

    // Options / status
    PLAYER_HOST_CALLBACK(setOption);
    PLAYER_HOST_CALLBACK(checkOption);
    PLAYER_HOST_CALLBACK(checkOption1);
    PLAYER_HOST_CALLBACK(checkOption2);
    PLAYER_HOST_CALLBACK(scStart);
    PLAYER_HOST_CALLBACK(scEnd);
    PLAYER_HOST_CALLBACK(getStatus);
    PLAYER_HOST_CALLBACK(isDead);
    PLAYER_HOST_CALLBACK(recalculateStat);
    PLAYER_HOST_CALLBACK(needStatusPoint);

    // Reset
    PLAYER_HOST_CALLBACK(resetStatus);
    PLAYER_HOST_CALLBACK(resetSkill);
    PLAYER_HOST_CALLBACK(resetFeel);
    PLAYER_HOST_CALLBACK(resetHate);

    // Display effects
    PLAYER_HOST_CALLBACK(message);
    PLAYER_HOST_CALLBACK(dispBottom);
    PLAYER_HOST_CALLBACK(showScript);
    PLAYER_HOST_CALLBACK(cutin);
    PLAYER_HOST_CALLBACK(emotion);
    PLAYER_HOST_CALLBACK(miscEffect);
    PLAYER_HOST_CALLBACK(soundEffect);
    PLAYER_HOST_CALLBACK(playBgm);
    PLAYER_HOST_CALLBACK(viewpoint);
    PLAYER_HOST_CALLBACK(showDigit);
    PLAYER_HOST_CALLBACK(hatEffect);

    // UI windows
    PLAYER_HOST_CALLBACK(openStorage);
    PLAYER_HOST_CALLBACK(openBank);
    PLAYER_HOST_CALLBACK(openMail);
    PLAYER_HOST_CALLBACK(openAuction);
    PLAYER_HOST_CALLBACK(openRefineUi);
    PLAYER_HOST_CALLBACK(openStylist);
    PLAYER_HOST_CALLBACK(openDressRoom);
    PLAYER_HOST_CALLBACK(openRoulette);
    PLAYER_HOST_CALLBACK(openQuestUi);
    PLAYER_HOST_CALLBACK(openEnchantGrade);
    PLAYER_HOST_CALLBACK(openLaphineSynthesis);
    PLAYER_HOST_CALLBACK(openLaphineUpgrade);
    PLAYER_HOST_CALLBACK(openItemEnchant);
    PLAYER_HOST_CALLBACK(openItemReform);
    PLAYER_HOST_CALLBACK(specialPopup);
    PLAYER_HOST_CALLBACK(openTips);
    PLAYER_HOST_CALLBACK(readBook);

    // Spirit balls
    PLAYER_HOST_CALLBACK(addSpiritBall);
    PLAYER_HOST_CALLBACK(delSpiritBall);
    PLAYER_HOST_CALLBACK(countSpiritBall);

    // Reputation / fame
    PLAYER_HOST_CALLBACK(getReputation);
    PLAYER_HOST_CALLBACK(setReputation);
    PLAYER_HOST_CALLBACK(addReputation);
    PLAYER_HOST_CALLBACK(getFame);
    PLAYER_HOST_CALLBACK(addFame);
    PLAYER_HOST_CALLBACK(getFameRank);

    // Marriage / family
    PLAYER_HOST_CALLBACK(marry);
    PLAYER_HOST_CALLBACK(divorce);
    PLAYER_HOST_CALLBACK(adopt);
    PLAYER_HOST_CALLBACK(getPartnerId);
    PLAYER_HOST_CALLBACK(getMotherId);
    PLAYER_HOST_CALLBACK(getFatherId);
    PLAYER_HOST_CALLBACK(getChildId);
    PLAYER_HOST_CALLBACK(isPartnerOn);

    // Permissions
    PLAYER_HOST_CALLBACK(permissionCheck);
    PLAYER_HOST_CALLBACK(permissionAdd);
    PLAYER_HOST_CALLBACK(permissionRemove);
    PLAYER_HOST_CALLBACK(guildHasPermission);

    // VIP / macro
    PLAYER_HOST_CALLBACK(vipStatus);
    PLAYER_HOST_CALLBACK(vipTime);
    PLAYER_HOST_CALLBACK(macroDetector);

    // Misc
    PLAYER_HOST_CALLBACK(charInfo);
    PLAYER_HOST_CALLBACK(readParam);
    PLAYER_HOST_CALLBACK(charId4Type);
    PLAYER_HOST_CALLBACK(charIp);
    PLAYER_HOST_CALLBACK(kick);
    PLAYER_HOST_CALLBACK(ignoreTimeout);
    PLAYER_HOST_CALLBACK(autoLoot);
    PLAYER_HOST_CALLBACK(hasAutoLoot);
    PLAYER_HOST_CALLBACK(jobCanEnterMap);
    PLAYER_HOST_CALLBACK(checkVending);
    PLAYER_HOST_CALLBACK(checkChatting);
    PLAYER_HOST_CALLBACK(checkIdle);
    PLAYER_HOST_CALLBACK(navigateTo);
    PLAYER_HOST_CALLBACK(clanJoin);
    PLAYER_HOST_CALLBACK(clanLeave);
    PLAYER_HOST_CALLBACK(cameraInfo);

#undef PLAYER_HOST_CALLBACK
};

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING
