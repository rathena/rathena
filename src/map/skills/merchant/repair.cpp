// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "repair.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillRepair::SkillRepair() : SkillImpl(NC_REPAIR) {
}

void SkillRepair::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if (sd == nullptr) {
		return;
	}

	if (!dstsd || !pc_ismadogear(dstsd)) {
		clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL_TOTARGET);
		return;
	}

	int32 hp = 0;
	switch (skill_lv) {
		case 1: hp = 4; break;
		case 2: hp = 7; break;
		case 3: hp = 13; break;
		case 4: hp = 17; break;
		case 5:
		default: hp = 23; break;
	}

	int32 heal = dstsd->status.max_hp * hp / 100;
	status_heal(target, heal, 0, 2);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, heal != 0);
}
