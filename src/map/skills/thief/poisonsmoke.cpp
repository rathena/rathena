// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "poisonsmoke.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillPoisonSmoke::SkillPoisonSmoke() : SkillImpl(GC_POISONSMOKE) {
}

void SkillPoisonSmoke::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( !(sc && sc->getSCE(SC_POISONINGWEAPON)) ) {
		if( sd )
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_GC_POISONINGWEAPON );
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, flag);
}
