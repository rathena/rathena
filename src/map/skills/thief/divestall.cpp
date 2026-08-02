// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "divestall.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillDivestAll::SkillDivestAll() : SkillImpl(ST_FULLSTRIP) {
}

void SkillDivestAll::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );

	bool i;

	//Special message when trying to use strip on FCP [Jobbie]
	if( sd && tsc && tsc->getSCE(SC_CP_WEAPON) && tsc->getSCE(SC_CP_HELM) && tsc->getSCE(SC_CP_ARMOR) && tsc->getSCE(SC_CP_SHIELD))
	{
		clif_gospel_info( *sd, 0x28 );
		return;
	}

	if( i = skill_strip_equip(src, target, getSkillId(), skill_lv) )
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv,i);

	//Nothing stripped.
	if( sd && !i )
		clif_skill_fail( *sd, getSkillId() );
}
