// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// EM_ELEMENTAL_BUSTER
class SkillElementalBuster : public SkillImpl {
public:
	SkillElementalBuster();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// EM_ELEMENTAL_BUSTER_FIRE
class SkillElementalBusterFire : public SkillImplRecursiveDamageSplash {
public:
	SkillElementalBusterFire();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
};


// EM_ELEMENTAL_BUSTER_GROUND
class SkillElementalBusterGround : public SkillImplRecursiveDamageSplash {
public:
	SkillElementalBusterGround();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
};


// EM_ELEMENTAL_BUSTER_POISON
class SkillElementalBusterPoison : public SkillImplRecursiveDamageSplash {
public:
	SkillElementalBusterPoison();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
};


// EM_ELEMENTAL_BUSTER_WATER
class SkillElementalBusterWater : public SkillImplRecursiveDamageSplash {
public:
	SkillElementalBusterWater();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
};


// EM_ELEMENTAL_BUSTER_WIND
class SkillElementalBusterWind : public SkillImplRecursiveDamageSplash {
public:
	SkillElementalBusterWind();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
};
