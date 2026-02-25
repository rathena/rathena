// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_castling.hpp"

SkillCastling::SkillCastling() : SkillImpl(HAMI_CASTLE) {
}

void SkillCastling::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	//[orn]
	if (src != target && rnd_chance(20 * skill_lv, 100)) {
		// Get one of the monsters targeting the player and set the homunculus as its new target
		if (block_list* tbl = battle_gettargeted(target); tbl != nullptr && tbl->type == BL_MOB) {
			if (unit_data* ud = unit_bl2ud(tbl); ud != nullptr) {
				unit_changetarget_sub(*ud, *src);
			}
		}

		int16 x = src->x, y = src->y;
		// Move homunculus
		if (unit_movepos(src, target->x, target->y, 0, false)) {
			clif_blown(src);
			// Move player
			if (unit_movepos(target, x, y, 0, false)) {
				clif_blown(target);
			}
			// Show the animation on the homunculus only
			clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
		}
	} else if (homun_data* hd = BL_CAST(BL_HOM, src); hd != nullptr && hd->master != nullptr) {
		clif_skill_fail(*hd->master, getSkillId());
	} else if (map_session_data* sd = BL_CAST(BL_PC, target); sd != nullptr) {
		clif_skill_fail(*sd, getSkillId());
	}
}
