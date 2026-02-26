// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "psychicwave.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillPsychicWave::SkillPsychicWave() : SkillImpl(SO_PSYCHIC_WAVE) {
}

void SkillPsychicWave::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 70 * skill_lv + 3 * sstatus->int_;
	RE_LVL_DMOD(100);
	if (sc && (sc->getSCE(SC_HEATER_OPTION) || sc->getSCE(SC_COOLER_OPTION) || sc->getSCE(SC_BLAST_OPTION) || sc->getSCE(SC_CURSED_SOIL_OPTION)))
		skillratio += 20;
}

void SkillPsychicWave::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1; // Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
