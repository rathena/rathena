// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcphantomthrust.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/unit.hpp"

SkillNpcPhantomThrust::SkillNpcPhantomThrust() : WeaponSkillImpl(NPC_PHANTOMTHRUST) {
}

void SkillNpcPhantomThrust::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	unit_setdir(src,map_calc_dir(src, target->x, target->y));
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	skill_blown(src,target,distance_bl(src,target)-1,unit_getdir(src),BLOWN_NONE);
	if( battle_check_target(src,target,BCT_ENEMY) > 0 )
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
