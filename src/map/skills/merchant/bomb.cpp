// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "bomb.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"

SkillBomb::SkillBomb() : SkillImpl(AM_DEMONSTRATION) {
}

void SkillBomb::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag|=1;

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillBomb::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 20 * skill_lv;
}

void SkillBomb::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
#ifdef RENEWAL
	skill_break_equip(src,target, EQP_WEAPON, 300 * skill_lv, BCT_ENEMY);
#else
	skill_break_equip(src,target, EQP_WEAPON, 100*skill_lv, BCT_ENEMY);
#endif
}
