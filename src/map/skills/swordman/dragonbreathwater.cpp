// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dragonbreathwater.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillDragonBreathWater::SkillDragonBreathWater() : SkillImplRecursiveDamageSplash(RK_DRAGONBREATH_WATER) {
}

void SkillDragonBreathWater::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_FREEZING,15,skill_lv,skill_get_time(getSkillId(),skill_lv));
}

void SkillDragonBreathWater::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);

	if( tsc && tsc->getSCE(SC_HIDING) )
		clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
	else {
		skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, flag);
	}
}
