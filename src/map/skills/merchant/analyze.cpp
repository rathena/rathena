// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "analyze.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillAnalyze::SkillAnalyze() : SkillImpl(NC_ANALYZE) {
}

void SkillAnalyze::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	sc_start(src,target,skill_get_sc(getSkillId()), 30 + 12 * skill_lv,skill_lv,skill_get_time(getSkillId(),skill_lv));
}
