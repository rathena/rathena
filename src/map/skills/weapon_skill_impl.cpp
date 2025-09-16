// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "weapon_skill_impl.hpp"

WeaponSkillImpl::WeaponSkillImpl(e_skill skill_id) : SkillImpl(skill_id) {
}

void WeaponSkillImpl::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, flag);
}
