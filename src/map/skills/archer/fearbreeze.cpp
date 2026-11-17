// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fearbreeze.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillFearBreeze::SkillFearBreeze() : StatusSkillImpl(RA_FEARBREEZE) {
}

void SkillFearBreeze::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
}
