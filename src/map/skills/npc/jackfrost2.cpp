// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "jackfrost2.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillJackFrost2::SkillJackFrost2() : SkillImplRecursiveDamageSplash(NPC_JACKFROST) {
}

void SkillJackFrost2::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillJackFrost2::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}

void SkillJackFrost2::calculateSkillRatio(const Damage*, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32) const {
	const status_change* tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_FREEZING)) {
		skillratio += 900 + 300 * skill_lv;
		RE_LVL_DMOD(100);
	} else {
		skillratio += 400 + 100 * skill_lv;
		RE_LVL_DMOD(150);
	}
}

void SkillJackFrost2::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32, enum damage_lv) const {
	sc_start(src, target, SC_FREEZE, 200, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
