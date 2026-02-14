// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spellfist.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSpellFist::SkillSpellFist() : SkillImpl(SO_SPELLFIST) {
}

void SkillSpellFist::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	sc_type type = skill_get_sc(getSkillId());

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	unit_skillcastcancel(src, 1);
	if (sd) {
		sc_start4(src, src, type, 100, skill_lv, sd->skill_id_old, sd->skill_lv_old, 0, skill_get_time(getSkillId(), skill_lv));
		sd->skill_id_old = sd->skill_lv_old = 0;
	}
}
