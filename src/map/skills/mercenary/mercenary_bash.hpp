// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../battle_skill.hpp"

#include "../../battle.hpp"

class SkillMercenaryBash : public Skill
{
public:
	SkillMercenaryBash();

	void calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const override;
	void modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const override;
};
