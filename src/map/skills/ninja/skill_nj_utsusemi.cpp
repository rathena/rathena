#include "skill_nj_utsusemi.hpp"

#include "../../clif.hpp"
#include "../../status.hpp"

SkillNJ_UTSUSEMI::SkillNJ_UTSUSEMI() : SkillImpl(NJ_UTSUSEMI) {
}

void SkillNJ_UTSUSEMI::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *bl, this->skill_id_, skill_lv,
						sc_start(src, bl, SC_UTSUSEMI, 100, skill_lv, skill_get_time(this->skill_id_, skill_lv)));
	status_change_end(bl, SC_NEN);
}
