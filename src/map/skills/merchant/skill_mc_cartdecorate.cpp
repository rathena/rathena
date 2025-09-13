#include "skill_mc_cartdecorate.hpp"

SkillMC_CARTDECORATE::SkillMC_CARTDECORATE() : SkillImpl(MC_CARTDECORATE) {
}

int32 SkillMC_CARTDECORATE::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *bl, MC_CARTDECORATE, skill_lv);
	if (sd) {
		clif_SelectCart(sd);
	}
	return 1; // Success
}
