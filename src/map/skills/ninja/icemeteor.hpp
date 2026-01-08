// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "map/skills/skill_impl.hpp"

#include "map/battle.hpp"

class SkillIceMeteor : public SkillImpl {
public:
	SkillIceMeteor();

	void applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type,
	                            damage_lv dmg_lv) const override;
};
