// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "talismanofmagician.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTalismanOfMagician::SkillTalismanOfMagician() : SkillImpl(SOA_TALISMAN_OF_MAGICIAN) {
}

void SkillTalismanOfMagician::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	map_session_data* dstsd = BL_CAST( BL_PC, target );
	sc_type type = skill_get_sc(getSkillId());

	if( dstsd != nullptr ){
		int16 index = dstsd->equip_index[EQI_HAND_R];

		if (index >= 0 && dstsd->inventory_data[index] != nullptr && dstsd->inventory_data[index]->type == IT_WEAPON) {
			clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv)));
			return;
		}
	}

	if( sd != nullptr ){
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_NEED_WEAPON );
	}
}
