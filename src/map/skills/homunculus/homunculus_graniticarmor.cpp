// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_graniticarmor.hpp"

#include "map/battle.hpp"
#include "map/homunculus.hpp"
#include "map/status.hpp"

SkillGraniticArmor::SkillGraniticArmor() : SkillImpl(MH_GRANITIC_ARMOR) {
}

void SkillGraniticArmor::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const sc_type type = skill_get_sc(getSkillId());
	homun_data* hd = BL_CAST(BL_HOM, src);

	if (hd) {
		block_list* s_bl = battle_get_master(src);
		if (s_bl) {
			sc_start2(src, s_bl, type, 100, skill_lv, hd->homunculus.level, skill_get_time(getSkillId(), skill_lv)); //start on master
		}
		sc_start2(src, target, type, 100, skill_lv, hd->homunculus.level, skill_get_time(getSkillId(), skill_lv));
	}
}
