#include "battle_skill_factory.hpp"

#include "./swordsman/swordsman.hpp"
#include "./mercenary/mercenary.hpp"

void BattleSkillFactory::registerAllSkills()
{
    Swordsman::register_skills();
    Mercenary::register_skills();
}