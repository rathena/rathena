// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fullprotection.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillFullProtection::SkillFullProtection() : SkillImpl(CR_FULLPROTECTION) {
}

void SkillFullProtection::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	map_session_data* dstsd = BL_CAST( BL_PC, target );

	uint32 equip[] = {EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP};
	int32 i_eqp, s = 0, skilltime = skill_get_time(getSkillId(),skill_lv);

	for (i_eqp = 0; i_eqp < 4; i_eqp++) {
		if( target->type != BL_PC || ( dstsd && pc_checkequip(dstsd,equip[i_eqp]) < 0 ) )
			continue;
		sc_start(src,target,(sc_type)(SC_CP_WEAPON + i_eqp),100,skill_lv,skilltime);
		s++;
	}
	if( sd && !s ){
		clif_skill_fail( *sd, getSkillId() );
		// Don't consume item requirements
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
}
