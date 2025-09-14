// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hiding.hpp"

#include "map/status.hpp"
#include "map/clif.hpp"

SkillHiding::SkillHiding() : SkillImpl(TF_HIDING) {
}

int32 SkillHiding::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct status_change *tsc = status_get_sc(bl);
	struct status_change_entry *tsce = (tsc != nullptr) ? tsc->getSCE(SC_HIDING) : nullptr;

	if (tsce) {
		clif_skill_nodamage(src, *bl, this->skill_id, -1, status_change_end(bl, SC_HIDING)); // Hide skill-scream animation.
		return 0;
	}
	clif_skill_nodamage(src, *bl, this->skill_id, -1, sc_start(src, bl, SC_HIDING, 100, skill_lv, skill_get_time(this->skill_id, skill_lv)));
	return 1;
}