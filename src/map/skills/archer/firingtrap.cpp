// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firingtrap.hpp"

#include "map/status.hpp"

SkillFiringTrap::SkillFiringTrap() : SkillImpl(RA_FIRINGTRAP) {
}

void SkillFiringTrap::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start4(src, target, SC_BURNING, 50 + skill_lv * 10, skill_lv, 1000, src->id, 0, skill_get_time2(getSkillId(), skill_lv));
}

void SkillFiringTrap::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
