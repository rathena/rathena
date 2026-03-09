// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "talismanofblacktortoise.hpp"

#include <config/const.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillTalismanOfBlackTortoise::SkillTalismanOfBlackTortoise() : SkillImpl(SOA_TALISMAN_OF_BLACK_TORTOISE) {
}

void SkillTalismanOfBlackTortoise::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 2150 + 1600 * skill_lv;
	skillratio += pc_checkskill(sd, SOA_TALISMAN_MASTERY) * 15 * skill_lv;
	skillratio += 5 * sstatus->spl;
	if (sc != nullptr && sc->getSCE(SC_T_FIFTH_GOD) != nullptr)
		skillratio += 150 + 500 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillTalismanOfBlackTortoise::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);

	if (sc != nullptr && sc->getSCE(SC_T_THIRD_GOD) != nullptr){
		sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	}
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
