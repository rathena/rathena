// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spiritrecovery.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSpiritRecovery::SkillSpiritRecovery() : SkillImpl(SO_EL_CURE) {
}

void SkillSpiritRecovery::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ) {
		s_elemental_data *ed = sd->ed;

		if( !ed )
			return;

		int32 s_hp = sd->battle_status.hp * 10 / 100;
		int32 s_sp = sd->battle_status.sp * 10 / 100;

		if( !status_charge(sd,s_hp,s_sp) ) {
			clif_skill_fail( *sd, getSkillId() );
			return;
		}

		status_heal(ed,s_hp,s_sp,3);
		clif_skill_nodamage(src,*ed,getSkillId(),skill_lv);
	}
}
