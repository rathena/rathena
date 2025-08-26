// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "decagi.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"

SkillDecreaseAgi::SkillDecreaseAgi() : SkillImpl(AL_DECAGI)
{
}

void SkillDecreaseAgi::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const
{
	status_change *tsc = status_get_sc(bl);
	sc_type type = skill_get_sc(getSkillId());
	status_data *sstatus = status_get_status_data(*src);

	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, sc_start(src, bl, type, (50 + skill_lv * 3 + (status_get_lv(src) + sstatus->int_) / 5), skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
