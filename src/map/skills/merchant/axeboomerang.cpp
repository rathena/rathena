// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "axeboomerang.hpp"

#include <config/core.hpp>

SkillAxeBoomerang::SkillAxeBoomerang() : WeaponSkillImpl(NC_AXEBOOMERANG) {
}

void SkillAxeBoomerang::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += 150 + 50 * skill_lv;
	if (sd) {
		int16 index = sd->equip_index[EQI_HAND_R];

		if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON) {
			skillratio += sd->inventory_data[index]->weight / 10;
		}
	}
	RE_LVL_DMOD(100);
}
