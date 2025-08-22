// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "endure.hpp"

#include "../../clif.hpp"
#include "../../status.hpp"

SkillEndure::SkillEndure() : WeaponSkillImpl(SM_ENDURE)
{
}

void SkillEndure::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const
{
	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, sc_start(src, bl, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
