// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "nyanggrass.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillNyangGrass::SkillNyangGrass() : SkillImpl(SU_NYANGGRASS) {
}

void SkillNyangGrass::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && pc_checkskill(sd, SU_SPIRITOFLAND)) {
		sc_start(src, src, SC_DORAM_MATK, 100, sd->status.base_level, skill_get_time(SU_SPIRITOFLAND, 1));
	}
	flag |= 1;
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}
