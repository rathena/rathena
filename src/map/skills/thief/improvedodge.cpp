// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "improvedodge.hpp"


SkillImproveDodge::SkillImproveDodge() : WeaponSkillImpl(TF_MISS) {
}

int32 SkillImproveDodge::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// TF_MISS doesn't have a no damage implementation
	return 0;
}