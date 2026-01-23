// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crimsonarrowattack.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillCrimsonArrowAttack::SkillCrimsonArrowAttack() : SkillImplRecursiveDamageSplash(AG_CRIMSON_ARROW_ATK) {
}

void SkillCrimsonArrowAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 750 * skill_lv + 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}
