// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// EM_SUMMON_ELEMENTAL_DILUVIO
class SkillSummonElementalDiluvio : public SkillImpl {
public:
	SkillSummonElementalDiluvio();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_COLD_FORCE
class SkillColdForce : public SkillImpl {
public:
	SkillColdForce();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_CRYSTAL_ARMOR
class SkillCrystalArmor : public SkillImpl {
public:
	SkillCrystalArmor();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_AGE_OF_ICE
class SkillAgeOfIce : public SkillImplRecursiveDamageSplash {
public:
	SkillAgeOfIce();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
	void splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};
