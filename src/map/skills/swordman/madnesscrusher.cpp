// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "madnesscrusher.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillMadnessCrusher::SkillMadnessCrusher() : SkillImplRecursiveDamageSplash(DK_MADNESS_CRUSHER) {
}

void SkillMadnessCrusher::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1000 + 3800 * skill_lv;
	skillratio += 10 * sstatus->pow;
	if (sd != nullptr) {
		int16 index = sd->equip_index[EQI_HAND_R];

		if (index >= 0 && sd->inventory_data[index] != nullptr) {
			skillratio += sd->inventory_data[index]->weight / 10 * sd->inventory_data[index]->weapon_level;
		}
	}
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_CHARGINGPIERCE_COUNT) && sc->getSCE(SC_CHARGINGPIERCE_COUNT)->val1 >= 10)
		skillratio *= 2;
}
