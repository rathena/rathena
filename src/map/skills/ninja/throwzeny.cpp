// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "throwzeny.hpp"

#include "map/skill.hpp"

SkillThrowZeny::SkillThrowZeny() : SkillImpl(NJ_ZENYNAGE) {
}

void SkillThrowZeny::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_attack(skill_get_type(this->skill_id_), src, src, target, this->skill_id_, skill_lv, tick, flag);
}
