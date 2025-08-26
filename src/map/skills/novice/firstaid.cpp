// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firstaid.hpp"
#include "../../status.hpp"
#include "../../pc.hpp"

SkillFirstAid::SkillFirstAid(e_skill skill_id) : SkillImpl(skill_id) {
}

void SkillFirstAid::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// NV_FIRSTAID heals a small amount of HP
	if (src && target == src) {
		// Heal 5 HP (basic first aid amount)
		status_heal(src, 5, 0, 0);
	}
}

void SkillFirstAid::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// NV_FIRSTAID has no damage effects
}

void SkillFirstAid::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// NV_FIRSTAID has no skill ratio modifications
}

void SkillFirstAid::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// NV_FIRSTAID has no hit rate modifications
}

void SkillFirstAid::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// NV_FIRSTAID has no additional effects
}
