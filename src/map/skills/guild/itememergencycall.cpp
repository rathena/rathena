// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "itememergencycall.hpp"

SkillItemEmergencyCall::SkillItemEmergencyCall() : SkillImpl(static_cast<e_skill>(GD_ITEMEMERGENCYCALL)) {
}

void SkillItemEmergencyCall::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int8 dx[9] = { -1, 1, 0, 0, -1, 1, -1, 1, 0 };
	int8 dy[9] = { 0, 0, 1, -1, 1, -1, -1, 1, 0 };
	uint8 j = 0, calls = 0, called = 0;
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = nullptr;
	auto g = sd ? sd->guild : guild_search(status_get_guild_id(src));

	// i don't know if it actually summons in a circle, but oh well. ;P
	if (!g) {
		return;
	}

	switch (skill_lv) {
		case 1: calls = 7; break;
		case 2: calls = 12; break;
		case 3: calls = 20; break;
		default: calls = 0; break;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	for (int32 i = 0; i < g->guild.max_member && (!calls || called < calls); i++, j++) {
		if (j > 8) {
			j = 0;
		}
		if ((dstsd = g->guild.member[i].sd) != nullptr && sd != dstsd && !dstsd->state.autotrade && !pc_isdead(dstsd)) {
			if (dstsd->status.disable_call) {
				continue;
			}
			if (map_getmapflag(dstsd->m, MF_NOWARP) && !map_flag_gvg2(dstsd->m)) {
				continue;
			}
			if (!pc_job_can_entermap((enum e_job)dstsd->status.class_, src->m, pc_get_group_level(dstsd))) {
				continue;
			}
			if (map_getcell(src->m, src->x + dx[j], src->y + dy[j], CELL_CHKNOREACH)) {
				dx[j] = dy[j] = 0;
			}
			if (!pc_setpos(dstsd, map_id2index(src->m), src->x + dx[j], src->y + dy[j], CLR_RESPAWN)) {
				called++;
			}
		}
	}
	if (sd) {
#ifdef RENEWAL
		skill_blockpc_start(*sd, getSkillId(), skill_get_cooldown(getSkillId(), skill_lv));
#else
		guild_block_skill(sd, skill_get_time2(getSkillId(), skill_lv));
#endif
	}
}
