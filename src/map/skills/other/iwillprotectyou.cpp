// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "iwillprotectyou.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillIWillProtectYou::SkillIWillProtectYou() : SkillImpl(WE_MALE) {
}

void SkillIWillProtectYou::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);

	uint8 hp_rate = abs(skill_get_hp_rate(getSkillId(), skill_lv));

	if (hp_rate && status_get_hp(src) > status_get_max_hp(src) / hp_rate) {
		int32 gain_hp = tstatus->max_hp * hp_rate / 100; // The earned is the same % of the target HP than it costed the caster. [Skotlex]

		clif_skill_nodamage(src,*target,getSkillId(),status_heal(target, gain_hp, 0, 0));
	}
}
