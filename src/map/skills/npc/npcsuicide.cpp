// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcsuicide.hpp"

#include "map/clif.hpp"

SkillNpcSuicide::SkillNpcSuicide() : SkillImpl(NPC_SUICIDE) {
}

void SkillNpcSuicide::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	status_kill(src); //When suiciding, neither exp nor drops is given.
}
