// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "focusedarrowstrike.hpp"

#include <config/const.hpp>
#include <config/core.hpp>

#include "map/status.hpp"

SkillFocusedArrowStrike::SkillFocusedArrowStrike() : SkillImplRecursiveDamageSplash(SN_SHARPSHOOTING) {
}

void SkillFocusedArrowStrike::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	if (src->type == BL_MOB) { // TODO: Did these formulas change in the renewal balancing?
		if (wd->miscflag & 2) // Splash damage bonus
			skillratio += -100 + 140 * skill_lv;
		else
			skillratio += 100 + 50 * skill_lv;
		return;
	}
#ifdef RENEWAL
	skillratio += -100 + 300 + 300 * skill_lv;
	RE_LVL_DMOD(100);
#else
	skillratio += 100 + 50 * skill_lv;
#endif
}

void SkillFocusedArrowStrike::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	if( flag&1 ) {
		status_change_end(src, SC_CAMOUFLAGE);
	}
#else
	flag |= 2; // Flag for specific mob damage formula
	skill_area_temp[1] = target->id;
	if (battle_config.skill_eightpath_algorithm) {
		//Use official AoE algorithm
		if (!(map_foreachindir(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
		   skill_get_splash(getSkillId(), skill_lv), skill_get_maxcount(getSkillId(), skill_lv), 0, splash_target(src),
		   skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY))) {
			flag &= ~2; // Only targets in the splash area are affected

			//These skills hit at least the target if the AoE doesn't hit
			skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
		}
	} else {
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_maxcount(getSkillId(), skill_lv), splash_target(src),
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	}
#endif
}
