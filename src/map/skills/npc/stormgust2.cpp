// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stormgust2.hpp"

#include "map/status.hpp"

SkillStormGust2::SkillStormGust2() : SkillImpl(NPC_STORMGUST2) {
}

void SkillStormGust2::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	if (skill_lv == 1)
		sc_start(src,target,SC_FREEZE,10,skill_lv,skill_get_time2(getSkillId(),skill_lv));
	else if (skill_lv == 2)
		sc_start(src,target,SC_FREEZE,7,skill_lv,skill_get_time2(getSkillId(),skill_lv));
	else
		sc_start(src,target,SC_FREEZE,3,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}

void SkillStormGust2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 200 * skill_lv;
}

void SkillStormGust2::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;	// Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
