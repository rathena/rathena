// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// WL_TETRAVORTEX
class SkillTetraVortex : public SkillImpl {
public:
	SkillTetraVortex();

	void castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// WL_TETRAVORTEX_GROUND
class SkillTetraVortexEarth : public SkillImpl {
public:
	SkillTetraVortexEarth();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
	void castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// WL_TETRAVORTEX_FIRE
class SkillTetraVortexFire : public SkillImpl {
public:
	SkillTetraVortexFire();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
	void castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// WL_TETRAVORTEX_WATER
class SkillTetraVortexWater : public SkillImpl {
public:
	SkillTetraVortexWater();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
	void castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// WL_TETRAVORTEX_WIND
class SkillTetraVortexWind : public SkillImpl {
public:
	SkillTetraVortexWind();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
	void castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};
