// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rageburst.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillRageBurst::SkillRageBurst() : WeaponSkillImpl(LG_RAGEBURST) {
}

void SkillRageBurst::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && sd->spiritball_old) {
		skillratio += -100 + 200 * sd->spiritball_old + (status_get_max_hp(src) - status_get_hp(src)) / 100;
	} else {
		skillratio += 2900 + (status_get_max_hp(src) - status_get_hp(src));
	}

	RE_LVL_DMOD(100);
}
