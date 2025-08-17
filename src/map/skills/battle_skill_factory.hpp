#pragma once

#include "skill.hpp"
#include "../skill.hpp"

/**
 * Factory class for creating and managing Skill instances
 */
class BattleSkillFactory
{
public:
    // Destructor
    ~BattleSkillFactory() = default;

    static void registerSkill(const e_skill skill_id, const std::shared_ptr<Skill>& skill);
    static void registerAllSkills();
};
