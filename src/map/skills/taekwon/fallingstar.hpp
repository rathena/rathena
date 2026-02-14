// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// SJ_FALLINGSTAR
class SkillFallingStar : public StatusSkillImpl {
public:
	SkillFallingStar();
};


// SJ_FALLINGSTAR_ATK2
class SkillFallingStarAttack : public WeaponSkillImpl {
public:
	SkillFallingStarAttack();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
	void castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// SJ_FALLINGSTAR_ATK2
class SkillFallingStarAttack2 : public SkillImplRecursiveDamageSplash {
public:
	SkillFallingStarAttack2();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
};
