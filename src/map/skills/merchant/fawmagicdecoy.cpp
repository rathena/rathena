// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fawmagicdecoy.hpp"

SkillFawMagicDecoy::SkillFawMagicDecoy() : SkillImpl(NC_MAGICDECOY) {
}

void SkillFawMagicDecoy::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick, int32&) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		clif_magicdecoy_list(*sd, skill_lv, x, y);
	}
}
