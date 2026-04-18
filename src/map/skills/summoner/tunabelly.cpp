// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tunabelly.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/status.hpp"

SkillTunaBelly::SkillTunaBelly() : SkillImpl(SU_TUNABELLY) {
}

void SkillTunaBelly::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	uint32 heal = 0;

	if (dstmd && (dstmd->mob_id == MOBID_EMPERIUM || status_get_class_(target) == CLASS_BATTLEFIELD))
		heal = 0;
	else if (status_get_hp(target) != status_get_max_hp(target))
		heal = ((2 * skill_lv - 1) * 10) * status_get_max_hp(target) / 100;
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	status_heal(target, heal, 0, 0);
}
