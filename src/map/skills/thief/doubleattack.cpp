// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "doubleattack.hpp"


SkillDoubleAttack::SkillDoubleAttack() : WeaponSkillImpl(TF_DOUBLE) {
}

void SkillDoubleAttack::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id, skill_lv, tick, flag);
}

int32 SkillDoubleAttack::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_DOUBLE doesn't have a no damage implementation
	return 0;
}