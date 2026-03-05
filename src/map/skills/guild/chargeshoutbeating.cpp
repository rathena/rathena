// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chargeshoutbeating.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"

SkillChargeShoutBeating::SkillChargeShoutBeating() : SkillImpl(static_cast<e_skill>(GD_CHARGESHOUT_BEATING)) {
}

void SkillChargeShoutBeating::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && sd->guild && map_blid_exists(sd->guild->chargeshout_flag_id)) {
		block_list* mob_bl = map_id2bl(sd->guild->chargeshout_flag_id);

		if (mob_bl != nullptr && pc_setpos(sd, map_id2index(mob_bl->m), mob_bl->x, mob_bl->y, CLR_RESPAWN) != SETPOS_OK) {
			clif_skill_fail(*sd, getSkillId());
		} else {
			clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		}
	} else if (sd) {
		clif_skill_fail(*sd, getSkillId());
	}
}
