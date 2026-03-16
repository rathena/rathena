// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_steelhorn.hpp"

#include "map/status.hpp"

SkillSteelHorn::SkillSteelHorn() : SkillImpl(MH_STAHL_HORN) {
}

void SkillSteelHorn::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillSteelHorn::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 1000 + 300 * skill_lv * status_get_lv(src) / 150 + sstatus->vit; // !TODO: Confirm VIT bonus
}

void SkillSteelHorn::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_STUN, 20 + 2 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
