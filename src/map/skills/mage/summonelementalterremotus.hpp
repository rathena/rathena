// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// EM_SUMMON_ELEMENTAL_TERREMOTUS
class SkillSummonElementalTerremotus : public SkillImpl {
public:
	SkillSummonElementalTerremotus();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_EARTH_CARE
class SkillEarthCare : public SkillImpl {
public:
	SkillEarthCare();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_STRONG_PROTECTION
class SkillStrongProtection : public SkillImpl {
public:
	SkillStrongProtection();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_EL_AVALANCHE
class SkillAvalanche : public SkillImplRecursiveDamageSplash {
public:
	SkillAvalanche();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
	void splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};
