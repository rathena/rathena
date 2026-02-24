// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_overedboost.hpp"

SkillOveredBoost::SkillOveredBoost() : SkillImpl(MH_OVERED_BOOST) {
}

void SkillOveredBoost::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const sc_type type = skill_get_sc(getSkillId());
	homun_data* hd = BL_CAST(BL_HOM, src);

	if (hd != nullptr && battle_get_master(src) != nullptr) {
		sc_start(src, battle_get_master(src), type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}
}
