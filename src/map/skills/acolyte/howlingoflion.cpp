// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "howlingoflion.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillHowlingOfLion::SkillHowlingOfLion() : SkillImplRecursiveDamageSplash(SR_HOWLINGOFLION) {
}

void SkillHowlingOfLion::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 500 * skill_lv;
	RE_LVL_DMOD(100);
}

int64 SkillHowlingOfLion::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	status_change_end(target, SC_SWINGDANCE);
	status_change_end(target, SC_SYMPHONYOFLOVER);
	status_change_end(target, SC_MOONLITSERENADE);
	status_change_end(target, SC_RUSHWINDMILL);
	status_change_end(target, SC_ECHOSONG);
	status_change_end(target, SC_HARMONIZE);
	status_change_end(target, SC_NETHERWORLD);
	status_change_end(target, SC_VOICEOFSIREN);
	status_change_end(target, SC_DEEPSLEEP);
	status_change_end(target, SC_SIRCLEOFNATURE);
	status_change_end(target, SC_GLOOMYDAY);
	status_change_end(target, SC_GLOOMYDAY_SK);
	status_change_end(target, SC_SONGOFMANA);
	status_change_end(target, SC_DANCEWITHWUG);
	status_change_end(target, SC_SATURDAYNIGHTFEVER);
	status_change_end(target, SC_LERADSDEW);
	status_change_end(target, SC_MELODYOFSINK);
	status_change_end(target, SC_BEYONDOFWARCRY);
	status_change_end(target, SC_UNLIMITEDHUMMINGVOICE);

	int32 sflag = flag|SD_ANIMATION;
	return SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, sflag);
}

void SkillHowlingOfLion::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	skill_castend_damage_id(src, target, getSkillId(), skill_lv, tick, flag);
}

int32 SkillHowlingOfLion::getSplashTarget(block_list* src) const {
	return splash_target(src);
}
