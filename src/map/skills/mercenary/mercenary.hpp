#pragma once

#include "../battle_skill_factory.hpp"
#include "mercenary_bash.hpp"

class Mercenary
{
public:
    static void register_skills()
    {
        BattleSkillFactory::registerSkill(MS_BASH, std::make_shared<MercenaryBashSkill>());
    }
};
