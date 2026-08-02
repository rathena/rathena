// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crossimpact.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"

SkillCrossImpact::SkillCrossImpact() : WeaponSkillImpl(GC_CROSSIMPACT) {
}

void SkillCrossImpact::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 1400 + 150 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillCrossImpact::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	uint8 dir = DIR_NORTHEAST;

	if (target->x != src->x || target->y != src->y)
		dir = map_calc_dir(target, src->x, src->y);	// dir based on target as we move player based on target location

	if (skill_check_unit_movepos(0, src, target->x + dirx[dir], target->y + diry[dir], 1, 1)) {
		clif_blown(src);
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	} else {
		if (sd)
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
	}
}
