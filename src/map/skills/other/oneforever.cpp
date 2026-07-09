// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "oneforever.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillOneForever::SkillOneForever() : SkillImpl(WE_ONEFOREVER) {
}

void SkillOneForever::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if (sd) {
		map_session_data *p_sd = pc_get_partner(sd);
		map_session_data *c_sd = pc_get_child(sd);

		if (!p_sd && !c_sd && !dstsd) { // Fail if no family members are found
			clif_skill_fail( *sd, getSkillId() );
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}
		if (map_flag_gvg2(target->m) || map_getmapflag(target->m, MF_BATTLEGROUND)) { // No reviving in WoE grounds!
			clif_skill_fail( *sd, getSkillId() );
			return;
		}
		if (status_isdead(*target)) {
			int32 per = 30, sper = 0;

			if (battle_check_undead(tstatus->race, tstatus->def_ele))
				return;
			if (tsc && tsc->getSCE(SC_HELLPOWER))
				return;
			if (map_getmapflag(target->m, MF_PVP) && dstsd->pvp_point < 0)
				return;
			if (dstsd->special_state.restart_full_recover)
				per = sper = 100;
			if ((dstsd == p_sd || dstsd == c_sd) && status_revive(target, per, sper)) // Only family members can be revived
				clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		}
	}
}
