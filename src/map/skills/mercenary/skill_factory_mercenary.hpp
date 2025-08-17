#pragma once

#include "../skill_factory.hpp"
#include "mercenary_bash.hpp"

class SkillFactoryMercenary : SkillFactory
{
public:
    static void registerSkills()
    {
        registerSkill(MS_BASH, std::make_shared<SkillMercenaryBash>());
    }
};
