// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../weapon_skill_impl.hpp"

#include "map/battle.hpp"

class SkillCounter : public WeaponSkillImpl {
public:
	SkillCounter();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const override;
};
