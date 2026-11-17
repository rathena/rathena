// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "illusionshadow.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/unit.hpp"

SkillIllusionShadow::SkillIllusionShadow() : SkillImpl(KO_ZANZOU) {
}

void SkillIllusionShadow::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if(sd){
		mob_data *md2 = mob_once_spawn_sub(src, src->m, src->x, src->y, status_get_name(*src), MOBID_ZANZOU, "", SZ_SMALL, AI_NONE);
		if( md2 )
		{
			md2->master_id = src->id;
			md2->special_state.ai = AI_ZANZOU;
			if( md2->deletetimer != INVALID_TIMER )
				delete_timer(md2->deletetimer, mob_timer_delete);
			md2->deletetimer = add_timer (gettick() + skill_get_time(getSkillId(), skill_lv), mob_timer_delete, md2->id, 0);
			mob_spawn( md2 );
			map_foreachinallrange(unit_changetarget, src, AREA_SIZE, BL_MOB, src, md2);
			clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
			skill_blown(src,target,skill_get_blewcount(getSkillId(),skill_lv),unit_getdir(target),BLOWN_NONE);
		}
	}
}
