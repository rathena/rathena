// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chainlightning.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

// WL_CHAINLIGHTNING
SkillChainLightning::SkillChainLightning() : SkillImpl(WL_CHAINLIGHTNING) {
}

void SkillChainLightning::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_addtimerskill(src, tick + status_get_amotion(src), target->id, 0, 0, WL_CHAINLIGHTNING_ATK, skill_lv, 0, 0);
}


// WL_CHAINLIGHTNING_ATK
SkillChainLightningAttack::SkillChainLightningAttack() : SkillImpl(WL_CHAINLIGHTNING_ATK) {
}

void SkillChainLightningAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += 400 + 100 * skill_lv;
	RE_LVL_DMOD(100);
	if (mflag > 0)
		skillratio += 100 * mflag;
}
