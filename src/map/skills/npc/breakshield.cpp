// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "breakshield.hpp"

#include "map/pc.hpp"

SkillBreakShield::SkillBreakShield() : WeaponSkillImpl(NPC_SHIELDBRAKE) {
}

void SkillBreakShield::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	skill_break_equip(src,target, EQP_SHIELD, 150*skill_lv, BCT_ENEMY);
}
