// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skillrepository.hpp"
#include "swordsman.hpp"

#include <memory>

SkillRepository skillRepository;

const Skill& SkillRepository::getSkill(e_skill skill_id) {
    auto skill = skill_db_.find(skill_id);
    if (skill == skill_db_.end()) {
        throw SkillNotFoundException{};
    }
    return *skill->second;
}

void SkillRepository::addSkill(e_skill skill_id, std::unique_ptr<Skill> skill) {
    skill_db_.emplace(skill_id, std::move(skill));
}

void init_skill_repository() {
    init_swordsman_skills(skillRepository);
}
