// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chargeshoutflag.hpp"

SkillChargeShoutFlag::SkillChargeShoutFlag() : SkillImpl(static_cast<e_skill>(GD_CHARGESHOUT_FLAG)) {
}

void SkillChargeShoutFlag::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && sd->guild && sd->state.gmaster_flag == 1) {
		mob_data* md = mob_once_spawn_sub(src, src->m, src->x, src->y, sd->guild->guild.name, MOBID_GUILD_SKILL_FLAG, nullptr, SZ_SMALL, AI_GUILD);

		if (md) {
			sd->guild->chargeshout_flag_id = md->id;
			md->master_id = src->id;

			if (md->deletetimer != INVALID_TIMER) {
				delete_timer(md->deletetimer, mob_timer_delete);
			}
			md->deletetimer = add_timer(gettick() + skill_get_time(getSkillId(), skill_lv), mob_timer_delete, md->id, 0);
			mob_spawn(md);
		}
	}
}
