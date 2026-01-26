// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "earthflower.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillEarthFlower::SkillEarthFlower() : SkillImpl(DR_EARTH_FLOWER) {
}

void SkillEarthFlower::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 100 * skill_lv;

	if (sc != nullptr && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
		skillratio += 5 * sstatus->int_;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_EARTH
	RE_LVL_DMOD(100);
}

void SkillEarthFlower::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
