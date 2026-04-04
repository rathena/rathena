// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "solidstomp.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

#include "groundbloom.hpp"

SkillSolidStomp::SkillSolidStomp() : SkillImplRecursiveDamageSplash(AT_SOLID_STOMP) {
}

void SkillSolidStomp::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	// Additional bonuses, given before damage calculation
	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));

	int64 heal = static_cast<int64>(status_get_max_hp(src)) * skill_lv / 100;
	if (heal > 0) {
		status_heal(src, heal, 0, 0);
	}

	// Skill damage
	this->castendDamageId(src, target, skill_lv, tick, flag);

	// Updates growth status and casts Ground Bloom if the conditions are met
	SkillGroundBloom::castGroundBloom(src, tick, 4);
}

void SkillSolidStomp::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 10400 + 800 * (skill_lv - 1);

	// (SPL and BaseLevel ratio do not depend on SC_TRUTH_OF_EARTH)
	skillratio += 20 * sstatus->spl;

	RE_LVL_DMOD(100);
}
