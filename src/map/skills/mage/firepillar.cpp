// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firepillar.hpp"

#include "map/unit.hpp"

SkillFirePillar::SkillFirePillar() : SkillImpl(WZ_FIREPILLAR) {
}

void SkillFirePillar::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	flag |= 1;

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillFirePillar::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += -60 + 20 * skill_lv; //20% MATK each hit
}

void SkillFirePillar::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	unit_set_walkdelay(target, tick, skill_get_time2(getSkillId(), skill_lv), 1);
}
