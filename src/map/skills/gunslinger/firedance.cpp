// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firedance.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillFireDance::SkillFireDance() : WeaponSkillImpl(RL_FIREDANCE) {
}

void SkillFireDance::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += 100 + 100 * skill_lv;
	skillratio += (sd ? pc_checkskill(sd, GS_DESPERADO) * 20 : 0);
	RE_LVL_DMOD(100);
}
