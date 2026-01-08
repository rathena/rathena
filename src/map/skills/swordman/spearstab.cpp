// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spearstab.hpp"

#include "map/unit.hpp"

SkillSpearStab::SkillSpearStab() : SkillImpl(KN_SPEARSTAB) {
}

void SkillSpearStab::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if(flag&1) {
		if (target->id==skill_area_temp[1])
			return;
		if (skill_attack(BF_WEAPON,src,src,target,getSkillId(), skill_lv, tick, SD_ANIMATION))
			skill_blown(src,target,skill_area_temp[2],-1,BLOWN_NONE);
	} else {
		int32 x=target->x,y=target->y,i,dir;
		dir = map_calc_dir(target,src->x,src->y);
		skill_area_temp[1] = target->id;
		skill_area_temp[2] = skill_get_blewcount(getSkillId(),skill_lv);
		// all the enemies between the caster and the target are hit, as well as the target
		if (skill_attack(BF_WEAPON,src,src,target, getSkillId(),skill_lv,tick,0))
			skill_blown(src,target,skill_area_temp[2],-1,BLOWN_NONE);
		for (i=0;i<4;i++) {
			map_foreachincell(skill_area_sub,target->m,x,y,BL_CHAR,
				src, getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
			x += dirx[dir];
			y += diry[dir];
		}
	}
}

void SkillSpearStab::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 20 * skill_lv;
}
