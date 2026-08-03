// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

class SkillPulseOfMadness : public StatusSkillImpl {
public:
	SkillPulseOfMadness();

	static void updateMadness(block_list* src);
};
