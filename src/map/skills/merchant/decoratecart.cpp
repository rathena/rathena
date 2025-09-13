// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "decoratecart.hpp"

#include "map/clif.hpp"

SkillDecorateCart::SkillDecorateCart() : SkillImpl(MC_CARTDECORATE) {
}

void SkillDecorateCart::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, MC_CARTDECORATE, skill_lv);
	if (sd) {
		clif_SelectCart(sd);
	}
}
