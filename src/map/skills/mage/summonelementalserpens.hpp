// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// EM_SUMMON_ELEMENTAL_SERPENS
class SkillSummonElementalSerpens : public SkillImpl {
public:
	SkillSummonElementalSerpens();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_DEEP_POISONING
class SkillDeepPoisoning : public SkillImpl {
public:
	SkillDeepPoisoning();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_POISON_SHIELD
class SkillPoisonShield : public SkillImpl {
public:
	SkillPoisonShield();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_DEADLY_POISON
class SkillDeadlyPoison : public SkillImplRecursiveDamageSplash {
public:
	SkillDeadlyPoison();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
	void splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};
