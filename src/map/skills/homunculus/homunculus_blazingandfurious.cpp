// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_blazingandfurious.hpp"

#include "map/status.hpp"

SkillBlazingAndFurious::SkillBlazingAndFurious() : SkillImplRecursiveDamageSplash(MH_BLAZING_AND_FURIOUS) {
}

void SkillBlazingAndFurious::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 80 * skill_lv * status_get_lv(src) / 100 + sstatus->str;
}
