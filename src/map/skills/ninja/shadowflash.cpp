// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shadowflash.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillShadowFlash::SkillShadowFlash() : SkillImplRecursiveDamageSplash(SS_KAGEGISSEN) {
}

void SkillShadowFlash::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 1500 + 950 * skill_lv;
	skillratio += pc_checkskill( sd, SS_KAGENOMAI ) * 150 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
	if (wd->miscflag & SKILL_ALTDMG_FLAG)
		skillratio = skillratio * 3 / 10;
}
