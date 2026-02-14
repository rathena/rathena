// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "piercingattack.hpp"

SkillPiercingAttack::SkillPiercingAttack() : WeaponSkillImpl(NPC_PIERCINGATT) {
}

void SkillPiercingAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += -25; // 75% base damage
}
