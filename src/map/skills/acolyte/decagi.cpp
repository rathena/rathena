// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "decagi.hpp"

#include "../../clif.hpp"
#include "../../status.hpp"

SkillDecreaseAgi::SkillDecreaseAgi() : SkillImpl(AL_DECAGI)
{
}

void SkillDecreaseAgi::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32& flag) const
{
	sc_type type = skill_get_sc(getSkillId());
	status_data *sstatus = status_get_status_data(*src);

	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, sc_start(src, bl, type, (50 + skill_lv * 3 + (status_get_lv(src) + sstatus->int_) / 5), skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
