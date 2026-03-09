// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fawremoval.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/status.hpp"

SkillFawRemoval::SkillFawRemoval() : SkillImpl(NC_DISJOINT) {
}

void SkillFawRemoval::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (target->type != BL_MOB) {
		return;
	}

	mob_data* md = map_id2md(target->id);
	if (md && md->mob_id >= MOBID_SILVERSNIPER && md->mob_id <= MOBID_MAGICDECOY_WIND) {
		status_kill(target);
	}
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
