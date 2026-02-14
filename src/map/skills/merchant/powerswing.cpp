// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "powerswing.hpp"

#include <config/core.hpp>

SkillPowerSwing::SkillPowerSwing() : WeaponSkillImpl(NC_POWERSWING) {
}

void SkillPowerSwing::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);

	skillratio += -100 + ((sstatus->str + sstatus->dex) / 2) + 300 + 100 * skill_lv;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_ABR_BATTLE_WARIOR)) {
		skillratio *= 2;
	}
}

void SkillPowerSwing::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick, int32, enum damage_lv) const {
	sc_start(src, target, SC_STUN, 10, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
