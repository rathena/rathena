// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "axetornado.hpp"

#include <config/core.hpp>

SkillAxeTornado::SkillAxeTornado() : WeaponSkillImpl(NC_AXETORNADO) {
}

void SkillAxeTornado::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32) const {
	const status_data* sstatus = status_get_status_data(*src);
	status_change* sc = status_get_sc(src);

	skillratio += -100 + 200 + 180 * skill_lv + sstatus->vit * 2;
	if (sc && sc->getSCE(SC_AXE_STOMP)) {
		skillratio += 380;
	}
	RE_LVL_DMOD(100);
}
