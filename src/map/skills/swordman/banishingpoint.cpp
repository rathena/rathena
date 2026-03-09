// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "banishingpoint.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillBanishingPoint::SkillBanishingPoint() : WeaponSkillImpl(LG_BANISHINGPOINT) {
}

void SkillBanishingPoint::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);

	skillratio += -100 + (100 * skill_lv);

	if (sd != nullptr) {
		skillratio += pc_checkskill(sd, SM_BASH) * 70;
	}

	if (sc != nullptr && sc->getSCE(SC_SPEAR_SCAR)) {
		skillratio += 800;
	}

	RE_LVL_DMOD(100);
}

void SkillBanishingPoint::modifyHitRate(int16& hitrate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hitrate += 5 * skill_lv;
}
