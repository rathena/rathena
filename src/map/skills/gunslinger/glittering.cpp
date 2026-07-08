// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "glittering.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillGlittering::SkillGlittering() : SkillImpl(GS_GLITTERING) {
}

void SkillGlittering::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	map_session_data *sd = BL_CAST(BL_PC, src);

	if (sd) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		if (rnd() % 100 < (20 + 10 * skill_lv))
			pc_addspiritball(sd, skill_get_time(getSkillId(), skill_lv), 10);
		else if (sd->spiritball > 0 && !pc_checkskill(sd, RL_RICHS_COIN))
			pc_delspiritball(sd, 1, 0);
	}
}
