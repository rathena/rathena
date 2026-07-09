// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "striking.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillStriking::SkillStriking() : SkillImpl(SO_STRIKING) {
}

void SkillStriking::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if (battle_check_target(src, target, BCT_SELF|BCT_PARTY) > 0) {
		int32 bonus = 0;

		if (dstsd) {
			int16 index = dstsd->equip_index[EQI_HAND_R];

			if (index >= 0 && dstsd->inventory_data[index] && dstsd->inventory_data[index]->type == IT_WEAPON)
				bonus = (20 * skill_lv) * dstsd->inventory_data[index]->weapon_level;
		}

		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start2(src,target, type, 100, skill_lv, bonus, skill_get_time(getSkillId(), skill_lv)));
	} else if (sd)
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_TOTARGET );
}
