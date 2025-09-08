#include "skill_mc_changecart.hpp"

SkillMC_CHANGECART::SkillMC_CHANGECART() : SkillImpl(MC_CHANGECART) {
}

int32 SkillMC_CHANGECART::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *bl, MC_CHANGECART, skill_lv);
	return 1; // Success
}
