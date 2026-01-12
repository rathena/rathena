// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "divestshield.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillDivestShield::SkillDivestShield() : SkillImpl(RG_STRIPSHIELD) {
}

void SkillDivestShield::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	bool i = skill_strip_equip(src, target, getSkillId(), skill_lv);

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,i);

	//Nothing stripped.
	if( sd && !i )
		clif_skill_fail( *sd, getSkillId() );
}
