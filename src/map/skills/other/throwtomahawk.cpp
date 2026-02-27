// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "throwtomahawk.hpp"

SkillThrowTomahawk::SkillThrowTomahawk() : WeaponSkillImpl(ITM_TOMAHAWK) {
}

void SkillThrowTomahawk::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
