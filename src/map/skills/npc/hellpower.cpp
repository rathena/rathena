// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hellpower.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillHellPower::SkillHellPower() : WeaponSkillImpl(NPC_HELLPOWER) {
}

void SkillHellPower::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv,
		sc_start(src, target, skill_get_sc(getSkillId()), skill_lv*20, skill_lv, skill_get_time2(getSkillId(), skill_lv)));
}
