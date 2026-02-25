// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "restoration.hpp"

SkillRestoration::SkillRestoration() : SkillImpl(static_cast<e_skill>(GD_RESTORE)) {
}

void SkillRestoration::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag & 1) {
		if (status_get_guild_id(src) == status_get_guild_id(target)) {
			clif_skill_nodamage(src, *target, AL_HEAL, status_percent_heal(target, 90, 90));
		}
	} else if (status_get_guild_id(src)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinallrange(skill_area_sub, src,
			skill_get_splash(getSkillId(), skill_lv), BL_PC,
			src, getSkillId(), skill_lv, tick, flag | BCT_GUILD | 1,
			skill_castend_nodamage_id);
		if (sd) {
#ifdef RENEWAL
			skill_blockpc_start(*sd, getSkillId(), skill_get_cooldown(getSkillId(), skill_lv));
#else
			guild_block_skill(sd, skill_get_time2(getSkillId(), skill_lv));
#endif
		}
	}
}
