// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_magmaflow.hpp"

#include <common/random.hpp>

#include "map/status.hpp"

SkillMagmaFlow::SkillMagmaFlow() : SkillImplRecursiveDamageSplash(MH_MAGMA_FLOW) {
}

void SkillMagmaFlow::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const sc_type type = skill_get_sc(getSkillId());

	sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillMagmaFlow::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if ((flag & 1) && ((rnd() % 100) > (3 * skill_lv))) {
		return; // chance to not trigger atk
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillMagmaFlow::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += -100 + (100 * skill_lv + 3 * status_get_lv(src)) * status_get_lv(src) / 120;
}
