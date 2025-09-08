#include "skill_nj_bunsinjyutsu.hpp"

#include "../../clif.hpp"
#include "../../status.hpp"

SkillNJ_BUNSINJYUTSU::SkillNJ_BUNSINJYUTSU() : SkillImpl(NJ_BUNSINJYUTSU) {
}

void SkillNJ_BUNSINJYUTSU::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	status_change_end(bl, SC_BUNSINJYUTSU); // on official recasting cancels existing mirror image [helvetica]
	clif_skill_nodamage(src, *bl, this->skill_id_, skill_lv,
						sc_start(src, bl, SC_BUNSINJYUTSU, 100, skill_lv, skill_get_time(this->skill_id_, skill_lv)));
	status_change_end(bl, SC_NEN);
}
