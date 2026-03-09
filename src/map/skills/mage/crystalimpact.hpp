// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// AG_CRYSTAL_IMPACT
class SkillCrystalImpact : public SkillImplRecursiveDamageSplash {
public:
	SkillCrystalImpact();

	void applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const override;
	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// AG_CRYSTAL_IMPACT_ATK
class SkillCrystalImpactAttack : public SkillImplRecursiveDamageSplash {
public:
	SkillCrystalImpactAttack();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
	int16 getSearchSize(block_list* src, uint16 skill_lv) const override;
	int16 getSplashSearchSize(block_list* src, uint16 skill_lv) const override;
};
