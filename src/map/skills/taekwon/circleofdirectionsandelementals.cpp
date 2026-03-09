// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "circleofdirectionsandelementals.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCircleOfDirectionsAndElementals::SkillCircleOfDirectionsAndElementals() : SkillImplRecursiveDamageSplash(SOA_CIRCLE_OF_DIRECTIONS_AND_ELEMENTALS) {
}

void SkillCircleOfDirectionsAndElementals::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 500 + 2000 * skill_lv;
	skillratio += pc_checkskill(sd, SOA_TALISMAN_MASTERY) * 15 * skill_lv;
	skillratio += pc_checkskill(sd, SOA_SOUL_MASTERY) * 15 * skill_lv;
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillCircleOfDirectionsAndElementals::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	skill_area_temp[0] = map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, BCT_ENEMY, skill_area_sub_count);
	sc_start(src,src,skill_get_sc(getSkillId()),100,skill_lv,skill_get_time(getSkillId(),skill_lv));

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
