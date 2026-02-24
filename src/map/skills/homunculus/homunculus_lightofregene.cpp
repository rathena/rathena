// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_lightofregene.hpp"

SkillLightOfRegene::SkillLightOfRegene() : SkillImpl(MH_LIGHT_OF_REGENE) {
}

void SkillLightOfRegene::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const sc_type type = skill_get_sc(getSkillId());
	homun_data* hd = BL_CAST(BL_HOM, src);

	if (hd == nullptr) {
		return;
	}

	block_list* s_bl = battle_get_master(src);
	if (s_bl != nullptr) {
		sc_start(src, s_bl, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}
	sc_start2(src, src, type, 100, skill_lv, hd->homunculus.level, skill_get_time(getSkillId(), skill_lv));
}
