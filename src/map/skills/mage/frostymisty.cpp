// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "frostymisty.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/status.hpp"

SkillFrostyMisty::SkillFrostyMisty() : SkillImpl(WL_FROSTMISTY) {
}

void SkillFrostyMisty::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 200 + 100 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillFrostyMisty::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Causes Freezing status through walls.
	sc_start(src, target, SC_FREEZING, 25 + 5 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
	sc_start(src, target, SC_MISTY_FROST, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	// Doesn't deal damage through non-shootable walls.
	if( !battle_config.skill_wall_check || (battle_config.skill_wall_check && path_search(nullptr,src->m,src->x,src->y,target->x,target->y,1,CELL_CHKWALL)) )
		skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag|SD_ANIMATION);
}

void SkillFrostyMisty::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = 0;

	// Cast center might be relevant later (e.g. for knockback direction)
	skill_area_temp[4] = x;
	skill_area_temp[5] = y;
	i = skill_get_splash(getSkillId(),skill_lv);
	map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR|BL_SKILL,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
}
