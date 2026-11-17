// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sequoiadust.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSequoiaDust::SkillSequoiaDust() : SkillImpl(ECL_SEQUOIADUST) {
}

void SkillSequoiaDust::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change_end(target, SC_STONE);
	status_change_end(target, SC_POISON);
	status_change_end(target, SC_CURSE);
	status_change_end(target, SC_BLIND);
	status_change_end(target, SC_ORCISH);
	status_change_end(target, SC_DECREASEAGI);

	clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), 1, DMG_SINGLE );
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
