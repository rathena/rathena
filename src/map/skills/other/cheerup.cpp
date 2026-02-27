// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cheerup.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCheerUp::SkillCheerUp() : StatusSkillImpl(WE_CHEERUP) {
}

void SkillCheerUp::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if (sd) {
		map_session_data *f_sd = pc_get_father(sd);
		map_session_data *m_sd = pc_get_mother(sd);

		if (!f_sd && !m_sd && !dstsd) { // Fail if no family members are found
			clif_skill_fail( *sd, getSkillId() );
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}
		if (flag&1) { // Buff can only be given to parents in 7x7 AoE around baby
			if (dstsd == f_sd || dstsd == m_sd)
				StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
		} else
			map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_PC, src, getSkillId(), skill_lv, tick, flag|BCT_ALL|1, skill_castend_nodamage_id);
	}
}
