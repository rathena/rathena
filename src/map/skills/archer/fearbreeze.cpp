// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fearbreeze.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillFearBreeze::SkillFearBreeze() : SkillImpl(RA_FEARBREEZE) {
}

void SkillFearBreeze::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src,target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
