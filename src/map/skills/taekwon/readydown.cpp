// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "readydown.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillReadyDown::SkillReadyDown() : SkillImpl(TK_READYDOWN) {
}

void SkillReadyDown::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 &flag) const {
	status_change_entry *tsce = status_get_sc(bl)->getSCE(SC_READYDOWN);

	if (tsce) {
		clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, status_change_end(bl, SC_READYDOWN));
		return;
	}

	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, sc_start(src, bl, SC_READYDOWN, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
