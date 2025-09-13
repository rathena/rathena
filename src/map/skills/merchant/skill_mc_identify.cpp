#include "skill_mc_identify.hpp"

SkillMC_IDENTIFY::SkillMC_IDENTIFY() : SkillImpl(MC_IDENTIFY) {
}

int32 SkillMC_IDENTIFY::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	if (sd) {
		clif_item_identify_list(sd);
		if (sd->menuskill_id != MC_IDENTIFY) {
			// failed, dont consume anything
			map_freeblock_unlock();
			return 1;
		} else {
			// consume sp only if succeeded
			struct s_skill_condition req = skill_get_requirement(sd, MC_IDENTIFY, skill_lv);
			status_zap(src, 0, req.sp);
		}
	}
	return 1; // Success
}
