// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "framen.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillFramen::SkillFramen() : SkillImplRecursiveDamageSplash(CD_FRAMEN) {
}

void SkillFramen::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 1300 * skill_lv;
	skillratio += 5 * pc_checkskill(sd, CD_FIDUS_ANIMUS) * skill_lv;
	skillratio += 5 * sstatus->spl;
	if (tstatus->race == RC_UNDEAD || tstatus->race == RC_DEMON)
		skillratio += 50 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillFramen::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
