// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_swordsman.hpp"

#include "bash.hpp"

void SkillFactorySwordsman::registerSkills(SkillDatabase& db) {
	registerSkill(db, SM_BASH, std::make_unique<SkillBash>());
}
