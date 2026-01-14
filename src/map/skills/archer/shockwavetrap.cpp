// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shockwavetrap.hpp"

#include "map/status.hpp"

SkillShockwaveTrap::SkillShockwaveTrap() : SkillImpl(HT_SHOCKWAVE) {
}

void SkillShockwaveTrap::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillShockwaveTrap::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_percent_damage(src, target, 0, -(15*skill_lv+5), false);
}
