// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "grandjudgement.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillGrandJudgement::SkillGrandJudgement() : SkillImplRecursiveDamageSplash(IG_GRAND_JUDGEMENT) {
}

void SkillGrandJudgement::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);
	int32 i;

	skillratio += -100 + 250 + 1500 * skill_lv + 10 * sstatus->pow;
	if (tstatus->race == RC_PLANT || tstatus->race == RC_INSECT)
		skillratio += 100 + 150 * skill_lv;
	RE_LVL_DMOD(100);
	if ((i = pc_checkskill_imperial_guard(sd, 3)) > 0)
		skillratio += skillratio * i / 100;
}

void SkillGrandJudgement::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
