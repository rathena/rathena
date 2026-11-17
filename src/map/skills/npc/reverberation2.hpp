// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// NPC_REVERBERATION
class SkillReverberation2 : public SkillImpl {
public:
	SkillReverberation2();

	void castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const override;
};


// NPC_REVERBERATION_ATK
class SkillReverberationAttack : public SkillImplRecursiveDamageSplash {
public:
	SkillReverberationAttack();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override;
	int32 getSplashTarget(block_list* src) const override;
	void splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};
