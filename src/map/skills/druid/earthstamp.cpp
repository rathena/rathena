// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "earthstamp.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

#include "skill_factory_druid.hpp"

SkillEarthStamp::SkillEarthStamp() : SkillImplRecursiveDamageSplash(KR_EARTH_STAMP) {
}

void SkillEarthStamp::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1000 + 70 * (skill_lv - 1);

	if (sc != nullptr && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
		skillratio += 4 * sstatus->int_;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_EARTH
	RE_LVL_DMOD(100);
}

void SkillEarthStamp::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	SkillFactoryDruid::try_gain_growth_stacks(src, tick, getSkillId());

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
