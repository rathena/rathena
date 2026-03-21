// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "falconassault.hpp"

SkillFalconAssault::SkillFalconAssault() : SkillImpl(SN_FALCONASSAULT) {
}

void SkillFalconAssault::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(skill_get_type(getSkillId()),src,src,target,getSkillId(),skill_lv,tick,flag);
}
