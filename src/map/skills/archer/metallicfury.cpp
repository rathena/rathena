// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "metallicfury.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillMetallicFury::SkillMetallicFury() : SkillImplRecursiveDamageSplash(TR_METALIC_FURY) {
}

void SkillMetallicFury::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* tsc = status_get_sc(target);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 3850 * skill_lv;
	// !Todo: skill affected by SPL (without SC_SOUNDBLEND) as well?
	if (tsc && tsc->getSCE(SC_SOUNDBLEND)) {
		skillratio += 800 * skill_lv;
		skillratio += 2 * pc_checkskill(sd, TR_STAGE_MANNER) * sstatus->spl;
	}
	RE_LVL_DMOD(100);
}
