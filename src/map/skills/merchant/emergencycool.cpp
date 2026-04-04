// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "emergencycool.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillEmergencyCool::SkillEmergencyCool() : SkillImpl(NC_EMERGENCYCOOL) {
}

void SkillEmergencyCool::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	if (sd == nullptr) {
		return;
	}

	struct s_skill_condition req = skill_get_requirement(sd, getSkillId(), skill_lv);
	int16 limit[] = { -45, -75, -105 };
	int32 i = 0;

	for (const auto& reqItem : req.eqItem) {
		if (pc_search_inventory(sd, reqItem) != -1) {
			break;
		}
		i++;
	}

	pc_overheat(*sd, limit[(i > 2) ? 2 : i]);
}
