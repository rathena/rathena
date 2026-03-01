// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "breakhelm.hpp"

#include "map/pc.hpp"

SkillBreakHelm::SkillBreakHelm() : WeaponSkillImpl(NPC_HELMBRAKE) {
}

void SkillBreakHelm::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	skill_break_equip(src,target, EQP_HELM, 150*skill_lv, BCT_ENEMY);
}
