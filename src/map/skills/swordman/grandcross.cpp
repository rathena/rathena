// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grandcross.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillGrandCross::SkillGrandCross() : SkillImpl(CR_GRANDCROSS) {
}

void SkillGrandCross::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag|=1;

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillGrandCross::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data* dstsd = BL_CAST(BL_PC, target);
	status_data* tstatus = status_get_status_data(*target);

	//Chance to cause blind status vs demon and undead element, but not against players
	if(!dstsd && (battle_check_undead(tstatus->race,tstatus->def_ele) || tstatus->race == RC_DEMON))
		sc_start(src,target,SC_BLIND,100,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}
