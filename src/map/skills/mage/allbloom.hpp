// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// AG_ALL_BLOOM
class SkillAllBloom : public SkillImpl {
public:
	SkillAllBloom();

	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
	void castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// AG_ALL_BLOOM_ATK
class SkillAllBloomAttack : public SkillImpl {
public:
	SkillAllBloomAttack();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
};


// AG_ALL_BLOOM_ATK2
class SkillAllBloomAttack2 : public SkillImpl {
public:
	SkillAllBloomAttack2();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
};
