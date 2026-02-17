// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pneumaticusprocella.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillPneumaticusProcella::SkillPneumaticusProcella() : SkillImpl(CD_PNEUMATICUS_PROCELLA) {
}

void SkillPneumaticusProcella::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag|=1;

	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}

void SkillPneumaticusProcella::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 150 + 2100 * skill_lv + 10 * sstatus->spl;
	skillratio += 3 * pc_checkskill( sd, CD_FIDUS_ANIMUS );
	if (tstatus->race == RC_UNDEAD || tstatus->race == RC_DEMON) {
		skillratio += 50 + 150 * skill_lv;
		skillratio += 2 * pc_checkskill( sd, CD_FIDUS_ANIMUS );
	}
	RE_LVL_DMOD(100);
}
