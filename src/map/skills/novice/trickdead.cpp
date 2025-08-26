// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "trickdead.hpp"
#include "../../status.hpp"
#include "../../skill.hpp"

SkillTrickDead::SkillTrickDead(e_skill skill_id) : SkillImpl(skill_id) {
}

void SkillTrickDead::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// NV_TRICKDEAD applies the trickdead status effect
	if (src && target == src) {
		// Apply SC_TRICKDEAD status effect
		sc_start(src, src, SC_TRICKDEAD, 100, skill_lv, skill_get_time(NV_TRICKDEAD, skill_lv));
	}
}

void SkillTrickDead::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// NV_TRICKDEAD has no damage effects
}

void SkillTrickDead::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	// NV_TRICKDEAD has no skill ratio modifications
}

void SkillTrickDead::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	// NV_TRICKDEAD has no hit rate modifications
}

void SkillTrickDead::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// NV_TRICKDEAD has no additional effects
}
