#pragma once

#include "skill.hpp"
#include "../skill.hpp"

/**
 * Factory class for creating and managing Skill instances
 */
class SkillFactory
{
public:
    // Destructor
    ~SkillFactory() = default;

    inline static void registerSkill(const e_skill skill_id, const std::shared_ptr<Skill>& skill);
    static void registerAllSkills();
    static void registerSkills();
};
