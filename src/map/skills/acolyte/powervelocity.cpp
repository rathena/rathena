// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "powervelocity.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillPowerVelocity::SkillPowerVelocity() : SkillImpl(SR_POWERVELOCITY) {
}

void SkillPowerVelocity::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if (!dstsd)
		return;

	if (sd && dstsd->spiritball <= 5) {
		for (int32 i = 0; i <= 5; i++) {
			pc_addspiritball(dstsd, skill_get_time(MO_CALLSPIRITS, pc_checkskill(sd, MO_CALLSPIRITS)), i);
			pc_delspiritball(sd, sd->spiritball, 0);
		}
	}
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
