// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "readyturn.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillReadyTurn::SkillReadyTurn() : SkillImpl(TK_READYTURN) {
}

void SkillReadyTurn::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32 &flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change_entry *tsce = status_get_sc(bl)->getSCE(type);

	if (tsce) {
		clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, status_change_end(bl, type));
		return;
	}

	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
