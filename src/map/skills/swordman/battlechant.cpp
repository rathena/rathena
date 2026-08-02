// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "battlechant.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillBattleChant::SkillBattleChant() : SkillImpl(PA_GOSPEL) {
}

void SkillBattleChant::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change* sc = status_get_sc(src);
	status_change_entry *sce = (sc && type != SC_NONE)?sc->getSCE(type):nullptr;

	if (sce && sce->val4 == BCT_SELF)
	{
		status_change_end(src, SC_GOSPEL);
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	else
	{
		std::shared_ptr<s_skill_unit_group> sg = skill_unitsetting(src,getSkillId(),skill_lv,src->x,src->y,0);
		if (!sg) return;
		if (sce)
			status_change_end(src, type); //Was under someone else's Gospel. [Skotlex]
		sc_start4(src,src,type,100,skill_lv,0,sg->group_id,BCT_SELF,skill_get_time(getSkillId(),skill_lv));
		clif_skill_poseffect( *src, getSkillId(), skill_lv, 0, 0, tick ); // PA_GOSPEL music packet
	}
}
