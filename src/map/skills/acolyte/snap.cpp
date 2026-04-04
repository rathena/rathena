// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "snap.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/unit.hpp"

SkillSnap::SkillSnap() : SkillImpl(MO_BODYRELOCATION) {
}

void SkillSnap::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (unit_movepos(src, x, y, 2, 1)) {
#if PACKETVER >= 20111005
		clif_snap(src, src->x, src->y);
#else
		clif_skill_poseffect( *src, getSkillId(), skill_lv, src->x, src->y, tick );
#endif
		if (sd)
			skill_blockpc_start (*sd, MO_EXTREMITYFIST, 2000);
	}
}
