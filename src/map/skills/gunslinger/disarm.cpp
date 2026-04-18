// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "disarm.hpp"

#include "map/clif.hpp"

SkillDisarm::SkillDisarm() : WeaponSkillImpl(GS_DISARM) {
}

void SkillDisarm::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	skill_strip_equip(src, target, getSkillId(), skill_lv);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
