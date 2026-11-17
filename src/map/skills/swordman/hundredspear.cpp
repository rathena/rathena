// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hundredspear.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillHundredSpear::SkillHundredSpear() : SkillImplRecursiveDamageSplash(RK_HUNDREDSPEAR) {
}

void SkillHundredSpear::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 600 + 200 * skill_lv;
	if (sd)
		skillratio += 50 * pc_checkskill(sd,LK_SPIRALPIERCE);
	if (sc) {
		if( sc->getSCE( SC_DRAGONIC_AURA ) ){
			skillratio += sc->getSCE( SC_DRAGONIC_AURA )->val1 * 160;
		}

		if (sc->getSCE(SC_CHARGINGPIERCE_COUNT) && sc->getSCE(SC_CHARGINGPIERCE_COUNT)->val1 >= 10)
			skillratio *= 2;
	}
	RE_LVL_DMOD(100);
}
