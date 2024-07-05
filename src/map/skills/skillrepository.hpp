// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include <memory>

#include "skill.hpp"
#include "map/skill.hpp"

class SkillRepository {
public:
	class SkillNotFoundException : public std::exception {};

	const Skill& getSkill(e_skill skill_id);

	void addSkill(e_skill skill_id, std::unique_ptr<Skill> skill);

private:
	std::unordered_map<e_skill, std::unique_ptr<Skill>> skill_db_;
};

extern SkillRepository skillRepository;

void init_skill_repository();