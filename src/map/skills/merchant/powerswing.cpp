// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "powerswing.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillPowerSwing::SkillPowerSwing() : WeaponSkillImpl(NC_POWERSWING) {
}

void SkillPowerSwing::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target, SC_STUN, 10, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillPowerSwing::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);

	// According to current sources, only the str + dex gets modified by level [Akinari]
	skillratio += -100 + ((sstatus->str + sstatus->dex)/ 2) + 300 + 100 * skill_lv;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_ABR_BATTLE_WARIOR)) {
		skillratio *= 2;
	}
}
