// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "netrepair.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillNetRepair::SkillNetRepair() : SkillImpl(ABR_NET_REPAIR) {
}

void SkillNetRepair::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);

	if (flag & 1) {
		int32 heal_amount = tstatus->max_hp * 10 / 100;
		clif_skill_nodamage(nullptr, *target, AL_HEAL, heal_amount);
		status_heal(target, heal_amount, 0, 0);
	} else {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ALLY | SD_SPLASH | 1, skill_castend_nodamage_id);
	}
}
