// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "trample.hpp"

#include "map/skill.hpp"
#include "map/status.hpp"

SkillTrample::SkillTrample() : SkillImpl(LG_TRAMPLE) {
}

void SkillTrample::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_damage(*src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE);

	if (rnd() % 100 < (25 + 25 * skill_lv)) {
		skill_trample_destroy_traps(target, skill_lv, tick);
	}

	status_change_end(target, SC_SV_ROOTTWIST);
}
