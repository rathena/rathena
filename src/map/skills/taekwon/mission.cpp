// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mission.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"

SkillMission::SkillMission() : SkillImpl(TK_MISSION) {
}

void SkillMission::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd) {
		if (sd->mission_mobid && (sd->mission_count || rnd() % 100)) {
			// Cannot change target when already have one
			clif_mission_info(sd, sd->mission_mobid, sd->mission_count);
			clif_skill_fail(*sd, getSkillId());
			return;
		}

		int32 id = mob_get_random_id(MOBG_TAEKWON_MISSION, RMF_NONE, 0);

		if (!id) {
			clif_skill_fail(*sd, getSkillId());
			return;
		}
		sd->mission_mobid = id;
		sd->mission_count = 0;
		pc_setglobalreg(sd, add_str(TKMISSIONID_VAR), id);
		clif_mission_info(sd, id, 0);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
