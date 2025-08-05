#include "battle_skill_factory.hpp"
#include "battle_skill_categories.hpp"
#include "skills/battle_skills.hpp"
#include "skill.hpp"

void BattleSkillFactory::registerAllSkills()
{
    registerWeaponSkills();
    registerMagicSkills();
    registerMiscSkills();
    registerPassiveSkills();
}

void BattleSkillFactory::registerWeaponSkills()
{
    registerSkill(SM_BASH, std::make_shared<BashSkill>());
    registerSkill(MS_BASH, std::make_shared<MercenaryBashSkill>());
}

// Register magic skills
void BattleSkillFactory::registerMagicSkills()
{
    // MG_FIREBOLT, MG_COLDBOLT etc
}

// Register misc skills
void BattleSkillFactory::registerMiscSkills()
{
    // NJ_ZENYNAGE, MO_EXTREMITYFIST etc
}

// Register passive skills
void BattleSkillFactory::registerPassiveSkills()
{
    // SM_SWORD, SM_TWOHAND etc
}
