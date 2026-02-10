// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulvulcanstrike.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillSoulVulcanStrike::SkillSoulVulcanStrike() : SkillImplRecursiveDamageSplash(AG_SOUL_VC_STRIKE) {
}

void SkillSoulVulcanStrike::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 300 * skill_lv + 3 * sstatus->spl;
	RE_LVL_DMOD(100);
}
