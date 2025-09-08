#include "skill_tk_dodge.hpp"

SkillTK_DODGE::SkillTK_DODGE() : SkillImpl(TK_DODGE) {
}

void SkillTK_DODGE::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct status_change_entry *tsce = status_get_sc(bl)->getSCE(SC_DODGE);
	
	if (tsce) {
		clif_skill_nodamage(src, *bl, this->skill_id, -1, status_change_end(bl, SC_DODGE)); // Hide skill-scream animation.
		return;
	}
	
	clif_skill_nodamage(src, *bl, this->skill_id, -1, sc_start(src, bl, SC_DODGE, 100, skill_lv, skill_get_time(this->skill_id, skill_lv)));
}
