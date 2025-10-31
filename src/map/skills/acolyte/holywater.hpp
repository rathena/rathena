// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder#pragma once

#include "../skill_impl.hpp"

class SkillHolyWater : public SkillImpl {
public:
	SkillHolyWater();
	
	void castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32& flag) const override;
};
