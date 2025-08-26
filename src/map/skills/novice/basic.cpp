// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "basic.hpp"

SkillBasic::SkillBasic(e_skill skill_id) : SkillImpl(skill_id) {
}

void SkillBasic::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// NV_BASIC has no special no-damage effects
}

void SkillBasic::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// NV_BASIC has no damage effects
}

void SkillBasic::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// NV_BASIC has no skill ratio modifications
}

void SkillBasic::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// NV_BASIC has no hit rate modifications
}

void SkillBasic::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// NV_BASIC has no additional effects
}
