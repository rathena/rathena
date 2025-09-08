#include "skill_tk_readystorm.hpp"

SkillTK_READYSTORM::SkillTK_READYSTORM() : SkillImpl(TK_READYSTORM) {
}

void SkillTK_READYSTORM::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct status_change_entry *tsce = status_get_sc(bl)->getSCE(SC_READYSTORM);
	
	if (tsce) {
		clif_skill_nodamage(src, *bl, this->skill_id, skill_lv, status_change_end(bl, SC_READYSTORM));
		return;
	}

	clif_skill_nodamage(src, *bl, this->skill_id, skill_lv, sc_start(src, bl, SC_READYSTORM, 100, skill_lv, skill_get_time(this->skill_id, skill_lv)));
}
