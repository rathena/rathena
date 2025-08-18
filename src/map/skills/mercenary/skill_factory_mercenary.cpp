// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_mercenary.hpp"

#include "mercenary_bash.hpp"

void SkillFactoryMercenary::registerSkills(SkillDatabase& db) {
	registerSkill(db, MS_BASH, std::make_unique<SkillMercenaryBash>());
}
