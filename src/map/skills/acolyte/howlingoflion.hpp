// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

class SkillHowlingOfLion : public SkillImplRecursiveDamageSplash {
public:
	SkillHowlingOfLion();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
	int64 splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
	void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
	int32 getSplashTarget(block_list* src) const override;
};
