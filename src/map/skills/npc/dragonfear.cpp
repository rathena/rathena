// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dragonfear.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillDragonFear::SkillDragonFear() : SkillImpl(NPC_DRAGONFEAR) {
}

void SkillDragonFear::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = 0;

	if (flag&1) {
		const enum sc_type sc[] = { SC_STUN, SC_SILENCE, SC_CONFUSION, SC_BLEEDING };
		int32 j;
		j = i = rnd()%ARRAYLENGTH(sc);
		while ( !sc_start2(src,target,sc[i],100,skill_lv,src->id,skill_get_time2(getSkillId(),i+1)) ) {
			i++;
			if ( i == ARRAYLENGTH(sc) )
				i = 0;
			if (i == j)
				break;
		}
	}
	else {
		skill_area_temp[2] = 0; //For SD_PREAMBLE
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		map_foreachinallrange(skill_area_sub, target,
			skill_get_splash(getSkillId(), skill_lv),BL_CHAR,
			src,getSkillId(),skill_lv,tick, flag|BCT_ENEMY|SD_PREAMBLE|1,
			skill_castend_nodamage_id);
	}
}
