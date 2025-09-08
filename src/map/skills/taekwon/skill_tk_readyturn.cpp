#include "skill_tk_readyturn.hpp"

SkillTK_READYTURN::SkillTK_READYTURN() : SkillImpl(TK_READYTURN) {
}

void SkillTK_READYTURN::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct status_change_entry *tsce = status_get_sc(bl)->getSCE(SC_READYTURN);
	
	if (tsce) {
		clif_skill_nodamage(src, *bl, this->skill_id, skill_lv, status_change_end(bl, SC_READYTURN));
		return;
	}

	clif_skill_nodamage(src, *bl, this->skill_id, skill_lv, sc_start(src, bl, SC_READYTURN, 100, skill_lv, skill_get_time(this->skill_id, skill_lv)));
}
