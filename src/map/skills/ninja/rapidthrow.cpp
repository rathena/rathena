// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rapidthrow.hpp"

#include <common/random.hpp>

#include "map/map.hpp"
#include "map/status.hpp"

SkillRapidThrow::SkillRapidThrow() : SkillImplRecursiveDamageSplash(KO_MUCHANAGE) {
}

void SkillRapidThrow::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* sstatus = status_get_status_data(*src);
	int32 i = skill_get_splash(getSkillId(),skill_lv);
	int32 rate = (100 - (1000 / (sstatus->dex + sstatus->luk) * 5)) * (skill_lv / 2 + 5) / 10;
	if( rate < 0 )
		rate = 0;
	skill_area_temp[0] = map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR,src,getSkillId(),skill_lv,tick,BCT_ENEMY,skill_area_sub_count);
	if( rnd()%100 < rate )
		map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
}
