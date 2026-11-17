// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "catnippowdering.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillCatnipPowdering::SkillCatnipPowdering() : SkillImpl(SU_CN_POWDERING) {
}

void SkillCatnipPowdering::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && pc_checkskill(sd, SU_SPIRITOFLAND)) {
		sc_start(src, src, SC_DORAM_FLEE2, 100, sd->status.base_level * 10 / 12, skill_get_time(SU_SPIRITOFLAND, 1));
	}
	flag |= 1;
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}
