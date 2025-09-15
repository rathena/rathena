// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_tk_readystorm.hpp"

SkillReadystorm::SkillReadystorm() : SkillImpl(TK_READYSTORM) {
}

void SkillReadystorm::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct status_change_entry *tsce = status_get_sc(bl)->getSCE(SC_READYSTORM);

	if (tsce) {
		clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, status_change_end(bl, SC_READYSTORM));
		return;
	}

	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, sc_start(src, bl, SC_READYSTORM, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
