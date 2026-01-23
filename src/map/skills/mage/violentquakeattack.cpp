// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "violentquakeattack.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillViolentQuakeAttack::SkillViolentQuakeAttack() : SkillImpl(AG_VIOLENT_QUAKE_ATK) {
}

void SkillViolentQuakeAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 200 + 1200 * skill_lv + 5 * sstatus->spl;
	// (climax buff applied with pc_skillatk_bonus)
	RE_LVL_DMOD(100);
}
