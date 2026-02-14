// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "blindingmist.hpp"

#include "map/status.hpp"

SkillBlindingMist::SkillBlindingMist() : SkillImpl(PF_FOGWALL) {
}

void SkillBlindingMist::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change* tsc = status_get_sc(target);

	if (src != target && (tsc == nullptr || tsc->getSCE(SC_DELUGE) == nullptr)) {
		sc_start(src, target, SC_BLIND, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	}
}
