// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "longingforfreedom.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillLongingForFreedom::SkillLongingForFreedom() : SkillImpl(CG_LONGINGFREEDOM) {
}

void SkillLongingForFreedom::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifndef RENEWAL
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(target);
	status_change_entry *tsce = (tsc != nullptr && type != SC_NONE) ? tsc->getSCE(type) : nullptr;

	if (tsc && !tsce && (tsce=tsc->getSCE(SC_DANCING)) && tsce->val4
		&& (tsce->val1&0xFFFF) != CG_MOONLIT) //Can't use Longing for Freedom while under Moonlight Petals. [Skotlex]
	{
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
			sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv)));
	}
#endif
}
