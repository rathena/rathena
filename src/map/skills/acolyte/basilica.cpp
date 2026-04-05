// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "basilica.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillBasilica::SkillBasilica() : StatusSkillImpl(HP_BASILICA) {
}

void SkillBasilica::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
#endif
}

void SkillBasilica::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifndef RENEWAL
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( status_change *sc = status_get_sc(src); sc && sc->getSCE(SC_BASILICA) ) {
		status_change_end(src, SC_BASILICA); // Cancel Basilica and return so requirement isn't consumed again
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	if( map_getcell(src->m, x, y, CELL_CHKLANDPROTECTOR) ) {
		if (sd)
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	
	// Create Basilica
	skill_clear_unitgroup(src);
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
	flag|=1;
#endif
}
