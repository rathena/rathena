#pragma once

#include "battle_skill_factory.hpp"
#include "battle_skill_categories.hpp"
#include "skills/battle_skills.hpp"
#include "skill.hpp"

BattleSkillFactory::BattleSkillFactory()
{
    registerAllSkills();
}

BattleSkillFactory &BattleSkillFactory::instance()
{
    static BattleSkillFactory instance;
    return instance;
}

std::shared_ptr<BattleSkill> BattleSkillFactory::getSkill(uint16 skill_id) const
{
    auto it = skill_db.find(skill_id);
    if (it != skill_db.end())
    {
        return it->second;
    }

    return nullptr;
}

bool BattleSkillFactory::hasSkill(uint16 skill_id) const
{
    return skill_db.find(skill_id) != skill_db.end();
}

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
    // Masteries and passive bonuses
    // register_skill<WeaponMasterySkill>(SM_SWORD);
    // register_skill<WeaponMasterySkill>(SM_TWOHAND);
    // register_skill<WeaponMasterySkill>(KN_SPEARMASTERY);
    // register_skill<WeaponMasterySkill>(AM_AXEMASTERY);
    // register_skill<WeaponMasterySkill>(PR_MACEMASTERY);
    // register_skill<WeaponMasterySkill>(MO_IRONHAND);
    // register_skill<WeaponMasterySkill>(AS_KATAR);
    // register_skill<WeaponMasterySkill>(BA_MUSICALLESSON);
    // register_skill<WeaponMasterySkill>(DC_DANCINGLESSON);
    // register_skill<WeaponMasterySkill>(SA_ADVANCEDBOOK);
}
