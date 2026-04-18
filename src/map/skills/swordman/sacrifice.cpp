// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sacrifice.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSacrifice::SkillSacrifice() : SkillImpl(CR_DEVOTION) {
}

void SkillSacrifice::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if (!sd) {
		return;
	}

	sc_type type = skill_get_sc(getSkillId());

	int32 count, lv;
	if( !dstsd )
	{ // Only players can be devoted
		clif_skill_fail( *sd, getSkillId() );
		return;
	}

	if( (lv = status_get_lv(src) - dstsd->status.base_level) < 0 )
		lv = -lv;
	if( lv > battle_config.devotion_level_difference || // Level difference requeriments
		(dstsd->sc.getSCE(type) && dstsd->sc.getSCE(type)->val1 != src->id) || // Cannot Devote a player devoted from another source
		(dstsd->class_&MAPID_SECONDMASK) == MAPID_CRUSADER || // Crusader Cannot be devoted
		(dstsd->sc.getSCE(SC_HELLPOWER))) // Players affected by SC_HELLPOWER cannot be devoted.
	{
		clif_skill_fail( *sd, getSkillId() );
		return;
	}

	int32 i = 0;
	count = min(skill_lv,MAX_DEVOTION);

	ARR_FIND(0, count, i, sd->devotion[i] == target->id );
	if( i == count )
	{
		ARR_FIND(0, count, i, sd->devotion[i] == 0 );
		if( i == count )
		{ // No free slots, skill Fail
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
	}

	sd->devotion[i] = target->id;

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv,
		sc_start4(src, target, type, 10000, src->id, i, skill_get_range2(src, getSkillId(), skill_lv, true), 0, skill_get_time2(getSkillId(), skill_lv)));
	clif_devotion(src, nullptr);
}
