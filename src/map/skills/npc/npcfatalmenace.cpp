// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcfatalmenace.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillNpcFatalMenace::SkillNpcFatalMenace() : WeaponSkillImpl(NPC_FATALMENACE) {
}

void SkillNpcFatalMenace::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// todo should it teleport the target ?
	if( flag&1 )
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	else {
		int16 x, y;
		map_search_freecell(src, 0, &x, &y, -1, -1, 0);
		// Destination area
		skill_area_temp[4] = x;
		skill_area_temp[5] = y;
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), splash_target(src), src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
		skill_addtimerskill(src,tick + 800,src->id,x,y,getSkillId(),skill_lv,0,flag); // To teleport Self
		clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	}
}
