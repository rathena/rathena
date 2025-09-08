#include "skill_gs_increasing.hpp"

SkillGS_INCREASING::SkillGS_INCREASING() : WeaponSkillImpl(GS_INCREASING) {
}

void SkillGS_INCREASING::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	// This is from skill_castend_nodamage_id.cpp
	// Note: GS_INCREASING was in a case fallthrough with GS_ADJUSTMENT
	// The actual implementation would be in the base WeaponSkillImpl class
	// We're just removing the case from the switch statement
}
