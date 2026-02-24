// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_holypole.hpp"

#include "map/status.hpp"

SkillHolyPole::SkillHolyPole() : SkillImplRecursiveDamageSplash(MH_HEILIGE_STANGE) {
}

void SkillHolyPole::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 1500 + 250 * skill_lv * status_get_lv(src) / 150 + sstatus->vit; // !TODO: Confirm VIT bonus
}
