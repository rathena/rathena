// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

#include "map/battle.hpp"

class SkillReadyCounter : public SkillImpl {
public:
	SkillReadyCounter();

	void castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 &flag) const override;
};
