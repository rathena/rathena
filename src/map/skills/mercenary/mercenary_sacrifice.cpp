// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_sacrifice.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/mercenary.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMercenarySacrifice::SkillMercenarySacrifice() : SkillImpl(ML_DEVOTION) {
}

void SkillMercenarySacrifice::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	s_mercenary_data* mer = BL_CAST(BL_MER, src);
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* dstsd = BL_CAST(BL_PC, target);
	int32 i = 0;

	int32 lv;
	if( !dstsd || !mer )
	{ // Only players can be devoted
		return;
	}

	if( (lv = status_get_lv(src) - dstsd->status.base_level) < 0 )
		lv = -lv;
	if( lv > battle_config.devotion_level_difference || // Level difference requeriments
		(dstsd->sc.getSCE(type) && dstsd->sc.getSCE(type)->val1 != src->id) || // Cannot Devote a player devoted from another source
		mer != dstsd->md || // Mercenary only can devote owner
		(dstsd->class_&MAPID_SECONDMASK) == MAPID_CRUSADER || // Crusader Cannot be devoted
		(dstsd->sc.getSCE(SC_HELLPOWER))) // Players affected by SC_HELLPOWER cannot be devoted.
	{
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	i = 0;
	mer->devotion_flag = 1; // Mercenary Devoting Owner

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv,
		sc_start4(src, target, type, 10000, src->id, i, skill_get_range2(src, getSkillId(), skill_lv, true), 0, skill_get_time2(getSkillId(), skill_lv)));
	clif_devotion(src, nullptr);
}
