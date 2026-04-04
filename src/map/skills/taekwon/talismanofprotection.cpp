// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "talismanofprotection.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillTalismanOfProtection::SkillTalismanOfProtection() : SkillImpl(SOA_TALISMAN_OF_PROTECTION) {
}

void SkillTalismanOfProtection::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start2(src, target, type, 100, skill_lv, src->id, skill_get_time(getSkillId(), skill_lv)));
}
