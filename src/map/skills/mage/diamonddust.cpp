// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "diamonddust.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillDiamondDust::SkillDiamondDust() : SkillImpl(SO_DIAMONDDUST) {
}

void SkillDiamondDust::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* sc = status_get_sc(src);
	status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 2 * sstatus->int_ + 300 * pc_checkskill(sd, SA_FROSTWEAPON) + sstatus->int_ * skill_lv;
	RE_LVL_DMOD(100);
	if( sc && sc->getSCE(SC_COOLER_OPTION) )
		skillratio += (sd ? sd->status.job_level * 5 : 0);
}

void SkillDiamondDust::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change* sc = status_get_sc(src);
	map_session_data* sd = BL_CAST(BL_PC, src);
	int32 rate = 5 + 5 * skill_lv;

	if( sc && sc->getSCE(SC_COOLER_OPTION) )
		rate += (sd ? sd->status.job_level / 5 : 0);

	sc_start(src, target, SC_CRYSTALIZE, rate, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}
