// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dragonbreath.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

// RK_DRAGONBREATH
SkillDragonBreath::SkillDragonBreath() : SkillImplRecursiveDamageSplash(RK_DRAGONBREATH) {
}

void SkillDragonBreath::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start4(src,target,SC_BURNING,15,skill_lv,1000,src->id,0,skill_get_time(getSkillId(),skill_lv));
}

void SkillDragonBreath::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);

	if( tsc && tsc->getSCE(SC_HIDING) )
		clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
	else {
		skill_attack(BF_WEAPON, src, src, target, getSkillId(), skill_lv, tick, flag);
	}
}

void SkillDragonBreath::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
	const status_change* sc = status_get_sc(&src);

	if (sc != nullptr) {
		if (sc->hasSCE(SC_LUXANIMA)) // Lux Anima has priority over Giant Growth
			element = ELE_DARK;
		else if (sc->hasSCE(SC_GIANTGROWTH))
			element = ELE_HOLY;
	}
}


// RK_DRAGONBREATH_WATER
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

void SkillDragonBreathWater::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
	const status_change* sc = status_get_sc(&src);

	if (sc != nullptr) {
		if (sc->hasSCE(SC_LUXANIMA)) // Lux Anima has priority over Fighting Spirit
			element = ELE_NEUTRAL;
		else if (sc->hasSCE(SC_FIGHTINGSPIRIT))
			element = ELE_GHOST;
	}
}
