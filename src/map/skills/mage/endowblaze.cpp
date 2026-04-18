// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "endowblaze.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEndowBlaze::SkillEndowBlaze() : SkillImpl(SA_FLAMELAUNCHER) {
}

void SkillEndowBlaze::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST( BL_PC, src );
	map_session_data* dstsd = BL_CAST( BL_PC, target );

	if (dstsd && dstsd->status.weapon == W_FIST) {
		if (sd)
			clif_skill_fail( *sd, getSkillId() );
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv,false);
		return;
	}
#ifdef RENEWAL
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
#else
	// 100% success rate at lv4 & 5, but lasts longer at lv5
	if(!clif_skill_nodamage(src,*target,getSkillId(),skill_lv, sc_start(src,target,type,(60+skill_lv*10),skill_lv, skill_get_time(getSkillId(),skill_lv)))) {
		if (dstsd){
			int16 index = dstsd->equip_index[EQI_HAND_R];
			if (index != -1 && dstsd->inventory_data[index] && dstsd->inventory_data[index]->type == IT_WEAPON)
				pc_unequipitem(dstsd, index, 3); //Must unequip the weapon instead of breaking it [Daegaladh]
		}
		if (sd)
			clif_skill_fail( *sd, getSkillId() );
	}
#endif
}
