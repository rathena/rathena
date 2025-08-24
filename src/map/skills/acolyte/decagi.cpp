#include "decagi.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"

SkillAL_DECAGI::SkillAL_DECAGI() : SkillImpl(AL_DECAGI) {
}

void SkillAL_DECAGI::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct status_change *tsc = nullptr;
	enum sc_type type = skill_get_sc(skill_id);

	if (bl->type == BL_PC)
		tsc = &((TBL_PC*)bl)->sc;
	else if (bl->type == BL_MOB)
		tsc = &((TBL_MOB*)bl)->sc;

	clif_skill_nodamage(src, *bl, skill_id, skill_lv,
			sc_start(src,bl, type, (50 + skill_lv * 3 + (status_get_lv(src) + sstatus->int_)/5), skill_lv, skill_get_time(skill_id,skill_lv)));
}
