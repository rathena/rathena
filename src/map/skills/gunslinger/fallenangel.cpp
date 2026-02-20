// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fallenangel.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillFallenAngel::SkillFallenAngel() : SkillImpl(RL_FALLEN_ANGEL) {
}

void SkillFallenAngel::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	sc_type type = skill_get_sc(getSkillId());

	if (unit_movepos(src, x, y, 1, 1)) {
		clif_snap(src, src->x, src->y);
		sc_start(src, src, type, 100, getSkillId(), skill_get_time(getSkillId(), skill_lv));
	} else if (sd != nullptr) {
		clif_skill_fail(*sd, getSkillId());
	}
}
