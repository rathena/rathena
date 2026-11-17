// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spiritcontrol.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/pc.hpp"

SkillSpiritControl::SkillSpiritControl() : SkillImpl(SO_EL_CONTROL) {
}

void SkillSpiritControl::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ) {
		int32 mode;

		if( !sd->ed )
			return;

		if( skill_lv == 4 ) {// At level 4 delete elementals.
			elemental_delete(sd->ed);
			return;
		}
		switch( skill_lv ) {// Select mode bassed on skill level used.
			case 1: mode = EL_MODE_PASSIVE; break; // Standard mode.
			case 2: mode = EL_MODE_ASSIST; break;
			case 3: mode = EL_MODE_AGGRESSIVE; break;
		}
		if( !elemental_change_mode(sd->ed,mode) ) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	}
}
