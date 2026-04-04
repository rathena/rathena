// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vulcanarm.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillVulcanArm::SkillVulcanArm() : SkillImplRecursiveDamageSplash(NC_VULCANARM) {
}

void SkillVulcanArm::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change *sc = status_get_sc(&src);

	if (sc != nullptr && sc->hasSCE(SC_ABR_DUAL_CANNON))
		dmg.div_ = 2;
}

void SkillVulcanArm::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 230 * skill_lv + sstatus->dex; // !TODO: What's the DEX bonus?
	RE_LVL_DMOD(100);
}
