#include "blessing.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"
#include "../../party.hpp"

SkillAL_BLESSING::SkillAL_BLESSING() : SkillImpl(AL_BLESSING)
{
}

void SkillAL_BLESSING::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const
{
	struct map_session_data *sd = nullptr;
	struct status_change *tsc = nullptr;
	enum sc_type type = skill_get_sc(skill_id);

	if (src->type == BL_PC)
		sd = (TBL_PC *)src;

	if (bl->type == BL_PC)
		tsc = &((TBL_PC *)bl)->sc;
	else if (bl->type == BL_MOB)
		tsc = &((TBL_MOB *)bl)->sc;

	clif_skill_nodamage(src, *bl, skill_id, skill_lv);
	if (dstsd != nullptr && tsc && tsc->getSCE(SC_CHANGEUNDEAD))
	{
		if (tstatus->hp > 1)
			skill_attack(BF_MISC, src, src, bl, skill_id, skill_lv, tick, flag);
		break;
	}
	sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
}