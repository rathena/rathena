// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "earthgrave.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillEarthGrave::SkillEarthGrave() : SkillImpl(SO_EARTHGRAVE) {
}

void SkillEarthGrave::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 2 * sstatus->int_ + 300 * pc_checkskill(sd, SA_SEISMICWEAPON) + sstatus->int_ * skill_lv;
	RE_LVL_DMOD(100);
	if( sc && sc->getSCE(SC_CURSED_SOIL_OPTION) )
		skillratio += (sd ? sd->status.job_level * 5 : 0);
}

void SkillEarthGrave::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src, target, SC_BLEEDING, 5 * skill_lv, skill_lv, src->id, skill_get_time2(getSkillId(), skill_lv));
}
