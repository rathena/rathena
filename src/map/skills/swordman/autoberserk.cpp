// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "autoberserk.hpp"

#include "../../clif.hpp"
#include "../../status.hpp"

SkillAutoBerserk::SkillAutoBerserk() : WeaponSkillImpl(SM_AUTOBERSERK)
{
}

void SkillAutoBerserk::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const
{
	status_change *tsc = status_get_sc(bl);
	status_change_entry *tsce = (tsc && skill_get_sc(getSkillId()) != SC_NONE) ? tsc->getSCE(skill_get_sc(getSkillId())) : nullptr;

	int32 i;
	if (tsce)
		i = status_change_end(bl, skill_get_sc(getSkillId()));
	else
		i = sc_start(src, bl, skill_get_sc(getSkillId()), 100, skill_lv, 60000);
	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, i);
}
