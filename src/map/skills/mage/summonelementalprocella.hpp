// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// EM_SUMMON_ELEMENTAL_PROCELLA
class SkillSummonElementalProcella : public SkillImpl {
public:
	SkillSummonElementalProcella();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_GRACE_BREEZE
class SkillGraceBreeze : public SkillImpl {
public:
	SkillGraceBreeze();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_EYES_OF_STORM
class SkillEyesOfStorm : public SkillImpl {
public:
	SkillEyesOfStorm();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_STORM_WIND
class SkillStormWind : public SkillImplRecursiveDamageSplash {
public:
	SkillStormWind();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
	void splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};
