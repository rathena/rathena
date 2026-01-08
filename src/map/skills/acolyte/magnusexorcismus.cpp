// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "magnusexorcismus.hpp"

#include "map/status.hpp"

SkillMagnusExorcismus::SkillMagnusExorcismus() : SkillImpl(PR_MAGNUS) {
}

void SkillMagnusExorcismus::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;

	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}

void SkillMagnusExorcismus::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* tstatus = status_get_status_data(*target);

	if (battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON)
		base_skillratio += 30;
}
