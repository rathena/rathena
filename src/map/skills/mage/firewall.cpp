// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firewall.hpp"

#include "map/status.hpp"
#include "map/unit.hpp"

SkillFireWall::SkillFireWall() : SkillImpl(MG_FIREWALL) {
}

void SkillFireWall::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillFireWall::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio -= 50;
}

void SkillFireWall::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_data* tstatus = status_get_status_data(target);

	if (tstatus->def_ele == ELE_FIRE || battle_check_undead(tstatus->race, tstatus->def_ele)) {
		dmg.blewcount = 0; // No knockback

		// Fire and undead units hit by firewall cannot be stopped for 2 seconds
		if (unit_data* ud = unit_bl2ud(const_cast<block_list*>(&target)); ud != nullptr)
			ud->endure_tick = gettick() + 2000;
	}
}
