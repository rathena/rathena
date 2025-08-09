#pragma once

#include "../battle_skill_factory.hpp"
#include "bash.hpp"

class Swordsman
{
public:
    static void register_skills()
    {
        BattleSkillFactory::registerSkill(SM_BASH, std::make_shared<BashSkill>());
    }
};