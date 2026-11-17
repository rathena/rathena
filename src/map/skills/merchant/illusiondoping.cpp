// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "illusiondoping.hpp"

#include "map/status.hpp"

SkillIllusionDoping::SkillIllusionDoping() : SkillImplRecursiveDamageSplash(GN_ILLUSIONDOPING) {
}

void SkillIllusionDoping::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	if( sc_start(src,target,SC_ILLUSIONDOPING,100 - skill_lv * 10,skill_lv,skill_get_time(getSkillId(),skill_lv)) )
		sc_start(src,target,SC_HALLUCINATION,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
}
