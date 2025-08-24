#include "crucis.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"

SkillAL_CRUCIS::SkillAL_CRUCIS() : SkillImpl(AL_CRUCIS)
{
}

void SkillAL_CRUCIS::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const
{
	struct status_change *tsc = nullptr;
	enum sc_type type = skill_get_sc(skill_id);

	if (bl->type == BL_PC)
		tsc = &((TBL_PC *)bl)->sc;
	else if (bl->type == BL_MOB)
		tsc = &((TBL_MOB *)bl)->sc;

	if (flag & 1)
		sc_start(src, bl, type, 25 + skill_lv * 4 + status_get_lv(src) - status_get_lv(bl), skill_lv, skill_get_time(skill_id, skill_lv));
	else
	{
		map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR,
							  src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_nodamage_id);
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
	}
}
