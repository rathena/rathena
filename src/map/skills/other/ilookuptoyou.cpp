// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ilookuptoyou.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillILookUpToYou::SkillILookUpToYou() : SkillImpl(WE_FEMALE) {
}

void SkillILookUpToYou::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);

	uint8 sp_rate = abs(skill_get_sp_rate(getSkillId(), skill_lv));

	if (sp_rate && status_get_sp(src) > status_get_max_sp(src) / sp_rate) {
		int32 gain_sp = tstatus->max_sp * sp_rate / 100; // The earned is the same % of the target SP than it costed the caster. [Skotlex]

		clif_skill_nodamage(src,*target,getSkillId(),status_heal(target, 0, gain_sp, 0));
	}
}
