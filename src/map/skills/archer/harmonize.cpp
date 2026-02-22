// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "harmonize.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillHarmonize::SkillHarmonize() : SkillImpl(MI_HARMONIZE) {
}

void SkillHarmonize::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	int32 duration = skill_get_time(getSkillId(), skill_lv);

	if( src != target ) {
		clif_skill_nodamage(src, *src, getSkillId(), skill_lv, sc_start(src, src, type, 100, skill_lv, duration));
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, duration));
}
