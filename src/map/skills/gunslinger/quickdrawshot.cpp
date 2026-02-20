// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "quickdrawshot.hpp"

SkillQuickDrawShot::SkillQuickDrawShot() : SkillImpl(RL_QD_SHOT) {
}

void SkillQuickDrawShot::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_area_temp[1] = target->id;
	map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
	status_change_end(src, SC_QD_SHOT_READY);
	skill_area_temp[1] = 0;
}

void SkillQuickDrawShot::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* tsc = status_get_sc(target);

	if (skill_area_temp[1] == target->id || (tsc != nullptr && tsc->getSCE(SC_C_MARKER) != nullptr)) {
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	}
}
