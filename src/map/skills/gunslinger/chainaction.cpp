// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chainaction.hpp"

SkillChainAction::SkillChainAction() : WeaponSkillImpl(GS_CHAINACTION) {
}

void SkillChainAction::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	// For NPC used skill.
	dmg.type = DMG_MULTI_HIT;
}
