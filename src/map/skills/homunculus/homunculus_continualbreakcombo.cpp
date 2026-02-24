// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_continualbreakcombo.hpp"

SkillContinualBreakCombo::SkillContinualBreakCombo() : SkillImpl(MH_CBC) {
}

void SkillContinualBreakCombo::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 duration = max(skill_lv, (status_get_str(src) / 7 - status_get_str(target) / 10)) * 1000; //Yommy formula

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start4(src, target, SC_CBC, 100, skill_lv, src->id, 0, 0, duration));
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
