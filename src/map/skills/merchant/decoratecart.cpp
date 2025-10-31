// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "decoratecart.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillDecorateCart::SkillDecorateCart() : SkillImpl(MC_CARTDECORATE) {
}

void SkillDecorateCart::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data *sd = BL_CAST(BL_PC, src);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	if (sd) {
		clif_SelectCart(sd);
	}
}
