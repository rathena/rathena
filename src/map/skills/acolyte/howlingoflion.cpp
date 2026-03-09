// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "howlingoflion.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillHowlingOfLion::SkillHowlingOfLion() : WeaponSkillImpl(SR_HOWLINGOFLION) {
}

void SkillHowlingOfLion::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 500 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillHowlingOfLion::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
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
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, sflag);
}

void SkillHowlingOfLion::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = splash_target(src);

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
