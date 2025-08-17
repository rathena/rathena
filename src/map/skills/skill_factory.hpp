// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "battle_skill.hpp"

#include "../skill.hpp"

/**
 * Factory class for creating and managing Skill instances
 */
class SkillFactory
{
public:
	// Destructor
	~SkillFactory() = default;

	static void registerSkill(const e_skill skill_id, const std::shared_ptr<Skill>& skill);
	static void registerAllSkills();
	static void registerSkills();
};
