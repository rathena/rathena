// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_decreaseagi.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillMercenaryDecreaseAgi::SkillMercenaryDecreaseAgi() : SkillImpl(MER_DECAGI) {
}

void SkillMercenaryDecreaseAgi::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* sstatus = status_get_status_data(*src);
	sc_type type = skill_get_sc(getSkillId());

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv,
		sc_start(src,target, type, (50 + skill_lv * 3 + (status_get_lv(src) + sstatus->int_)/5), skill_lv, skill_get_time(getSkillId(),skill_lv)));
}
