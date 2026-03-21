// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stealth.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillStealth::SkillStealth() : SkillImpl(ST_CHASEWALK) {
}

void SkillStealth::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(target);
	status_change_entry *tsce = (tsc && type != SC_NONE)?tsc->getSCE(type):nullptr;

	if (tsce)
	{
		clif_skill_nodamage(src,*target,getSkillId(),-1,status_change_end(target, type)); //Hide skill-scream animation.
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	clif_skill_nodamage(src,*target,getSkillId(),-1,sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv)));
}
