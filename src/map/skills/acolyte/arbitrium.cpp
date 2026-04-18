// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "arbitrium.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillArbitrium::SkillArbitrium() : SkillImplRecursiveDamageSplash(CD_ARBITRIUM) {
}

void SkillArbitrium::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 950 * skill_lv;
	skillratio += 10 * sstatus->spl;	// TODO : spl ratio has changed ?
	skillratio += 35 * pc_checkskill(sd, CD_FIDUS_ANIMUS) * skill_lv;

	RE_LVL_DMOD(100);
}

// TODO : CD_ARBITRIUM_ATK is no longer used ?
SkillArbitriumAttack::SkillArbitriumAttack() : SkillImplRecursiveDamageSplash(CD_ARBITRIUM_ATK) {
}

void SkillArbitriumAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1750 * skill_lv + 10 * sstatus->spl;
	skillratio += 50 * pc_checkskill(sd, CD_FIDUS_ANIMUS) * skill_lv;
	RE_LVL_DMOD(100);
}
