// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vanishingslash.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillVanishingSlash::SkillVanishingSlash() : StatusSkillImpl(NJ_KASUMIKIRI) {
}

void SkillVanishingSlash::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 20 * skill_lv;
#else
	base_skillratio += 10 * skill_lv;
#endif
}

void SkillVanishingSlash::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// TODO: refactor into applyAdditionalEffects
	if (skill_attack(BF_WEAPON,src,src,target,getSkillId(),skill_lv,tick,flag) > 0)
		sc_start(src,src,SC_HIDING,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
}
