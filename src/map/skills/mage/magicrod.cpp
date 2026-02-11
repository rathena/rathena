// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "magicrod.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillMagicRod::SkillMagicRod() : SkillImpl(SA_MAGICROD) {
}

void SkillMagicRod::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

#ifdef RENEWAL
	clif_skill_nodamage(src,*src,SA_MAGICROD,skill_lv);
#endif
	sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
}
