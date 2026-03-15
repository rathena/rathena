// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "doubleattack.hpp"

SkillDoubleAttack::SkillDoubleAttack() : WeaponSkillImpl(TF_DOUBLE) {
}

void SkillDoubleAttack::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	// For NPC used skill.
	dmg.type = DMG_MULTI_HIT;
}
