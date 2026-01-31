// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "jupitelthunderstorm.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillJupitelThunderstorm::SkillJupitelThunderstorm() : SkillImplRecursiveDamageSplash(HN_JUPITEL_THUNDER_STORM) {
}

void SkillJupitelThunderstorm::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 1800 * skill_lv;
	skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 3 * skill_lv;
	skillratio += 3 * sstatus->spl;
	RE_LVL_DMOD(100);
	// After RE_LVL_DMOD calculation, HN_SELFSTUDY_SOCERY amplifies the skill ratio of HN_JUPITEL_THUNDER_STORM by (skill level)%
	skillratio += skillratio * pc_checkskill(sd, HN_SELFSTUDY_SOCERY) / 100;
	// SC_RULEBREAK increases the skill ratio after HN_SELFSTUDY_SOCERY
	if (sc && sc->getSCE(SC_RULEBREAK))
		skillratio += skillratio * 70 / 100;
}

void SkillJupitelThunderstorm::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
