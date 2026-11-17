// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "decreaseallstats.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillDecreaseAllStats::SkillDecreaseAllStats() : SkillImpl(NPC_ALL_STAT_DOWN) {
}

void SkillDecreaseAllStats::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change_start(src, target, skill_get_sc(getSkillId()), 10000, skill_lv, 0, 0, 0, skill_get_time(getSkillId(), skill_lv), SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
}
