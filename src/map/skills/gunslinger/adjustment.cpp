// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "adjustment.hpp"

SkillAdjustment::SkillAdjustment() : WeaponSkillImpl(GS_ADJUSTMENT) {
}

void SkillAdjustment::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// This is from skill_castend_nodamage_id.cpp
	// Note: GS_ADJUSTMENT was in a case fallthrough with GS_INCREASING
	// The actual implementation would be in the base WeaponSkillImpl class
	// We're just removing the case from the switch statement
}
