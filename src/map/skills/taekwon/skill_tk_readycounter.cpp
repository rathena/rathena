// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_tk_readycounter.hpp"

SkillReadycounter::SkillReadycounter() : SkillImpl(TK_READYCOUNTER) {
}

void SkillReadycounter::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct status_change_entry *tsce = status_get_sc(bl)->getSCE(SC_READYCOUNTER);

	if (tsce) {
		clif_skill_nodamage(src, *bl, this->skill_id, skill_lv, status_change_end(bl, SC_READYCOUNTER));
		return;
	}

	clif_skill_nodamage(src, *bl, this->skill_id, skill_lv, sc_start(src, bl, SC_READYCOUNTER, 100, skill_lv, skill_get_time(this->skill_id, skill_lv)));
}
