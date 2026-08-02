// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_poisonmist.hpp"

#include "map/status.hpp"

SkillPoisonMist::SkillPoisonMist() : SkillImpl(MH_POISON_MIST) {
}

void SkillPoisonMist::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;
	// Ammo should be deleted right away.
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}

void SkillPoisonMist::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 200 * skill_lv * status_get_lv(src) / 100 + sstatus->dex; // ! TODO: Confirm DEX bonus
}
