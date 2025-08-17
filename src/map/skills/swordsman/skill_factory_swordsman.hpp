#pragma once

#include "../skill_factory.hpp"
#include "bash.hpp"

class SkillFactorySwordsman : SkillFactory
{
public:
    static void registerSkills()
    {
        registerSkill(SM_BASH, std::make_shared<SkillBash>());
    }
};
