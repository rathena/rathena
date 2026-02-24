// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_silverveinrush.hpp"

#include "map/status.hpp"

SkillSilverveinRush::SkillSilverveinRush() : SkillImpl(MH_SILVERVEIN_RUSH) {
}

void SkillSilverveinRush::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillSilverveinRush::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 250 * skill_lv * status_get_lv(src) / 100 + sstatus->str; // !TODO: Confirm STR bonus
}
