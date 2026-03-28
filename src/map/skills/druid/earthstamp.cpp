// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "earthstamp.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

#include "groundbloom.hpp"

SkillEarthStamp::SkillEarthStamp() : SkillImplRecursiveDamageSplash(KR_EARTH_STAMP) {
}

void SkillEarthStamp::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 1000 + 70 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 4 * sstatus->int_;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_EARTH
	RE_LVL_DMOD(100);
}

void SkillEarthStamp::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Updates growth status and casts Ground Bloom if the conditions are met
	SkillGroundBloom::castGroundBloom(src, tick, 1);

	// Skill damage
	SkillImplRecursiveDamageSplash::castendPos2(src, x, y, skill_lv, tick, flag);
}
