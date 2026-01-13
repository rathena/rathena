// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "closeconfine.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillCloseConfine::SkillCloseConfine() : SkillImpl(RG_CLOSECONFINE) {
}

void SkillCloseConfine::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start4(src,target,type,100,skill_lv,src->id,0,0,skill_get_time(getSkillId(),skill_lv)));
}
