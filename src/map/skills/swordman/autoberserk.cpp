// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "autoberserk.hpp"

#include "../../clif.hpp"
#include "../../status.hpp"

SkillAutoBerserk::SkillAutoBerserk() : SkillImpl(SM_AUTOBERSERK)
{
}

void SkillAutoBerserk::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32& flag) const
{
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(bl);
	status_change_entry *tsce = (tsc) ? tsc->getSCE(type) : nullptr;

	int32 i;
	if (tsce)
		i = status_change_end(bl, type);
	else
		i = sc_start(src, bl, type, 100, skill_lv, 60000);
	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, i);
}
