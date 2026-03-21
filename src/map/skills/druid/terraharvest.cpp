// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "terraharvest.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

#include "groundbloom.hpp"

SkillTerraHarvest::SkillTerraHarvest() : SkillImplRecursiveDamageSplash(AT_TERRA_HARVEST) {
}

void SkillTerraHarvest::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 18000 + 500 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 15 * sstatus->spl;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_EARTH
	RE_LVL_DMOD(100);
}

void SkillTerraHarvest::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	// Updates growth status and casts Ground Bloom if the conditions are met
	SkillGroundBloom::castGroundBloom(src, tick, 2);

	// Skill damage
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
