// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "widefreeze.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillWideFreeze::SkillWideFreeze() : SkillImpl(NPC_WIDEFREEZE) {
}

void SkillWideFreeze::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	if (flag&1){
		switch ( type ) {
		case SC_BURNING:
			sc_start4(src,target,type,100,skill_lv,1000,src->id,0,skill_get_time2(getSkillId(),skill_lv));
			break;
		case SC_STONEWAIT:
			sc_start2(src,target,type,100,skill_lv,src->id,skill_get_time2(getSkillId(),skill_lv),skill_get_time(getSkillId(), skill_lv));
			break;
		default:
			sc_start2(src,target,type,100,skill_lv,src->id,skill_get_time2(getSkillId(),skill_lv));
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
