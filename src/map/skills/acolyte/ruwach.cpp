#include "ruwach.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"

SkillAL_RUWACH::SkillAL_RUWACH() : SkillImpl(AL_RUWACH) {
}

void SkillAL_RUWACH::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct status_change *tsc = nullptr;
	enum sc_type type = skill_get_sc(skill_id);

	if (bl->type == BL_PC)
		tsc = &((TBL_PC*)bl)->sc;
	else if (bl->type == BL_MOB)
		tsc = &((TBL_MOB*)bl)->sc;

	clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start2(src,bl,type,100,skill_lv,skill_id,skill_get_time(skill_id,skill_lv)));
}
