// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "indulge.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillIndulge::SkillIndulge() : SkillImpl(PF_HPCONVERSION) {
}

void SkillIndulge::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const status_data* sstatus = status_get_status_data(*src);
	map_session_data* sd = BL_CAST(BL_PC, src);
	int32 hp = sstatus->max_hp / 10;
	int32 sp = hp * skill_lv;

	if (!status_charge(src, hp, 0)) {
		if (sd != nullptr) {
			clif_skill_fail(*sd, getSkillId());
		}
		return;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	status_heal(target, 0, sp, 2);
}
