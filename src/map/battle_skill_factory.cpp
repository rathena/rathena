#include "battle_skill_factory.hpp"
#include "battle_skill_categories.hpp"
#include "battle_skills.hpp"
#include "skill.hpp"

BattleSkillFactory::BattleSkillFactory()
{
    register_all_skills();
}

BattleSkillFactory &BattleSkillFactory::instance()
{
    static BattleSkillFactory instance;
    return instance;
}

std::shared_ptr<BattleSkill> BattleSkillFactory::get_skill(uint16 skill_id) const
{
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = skill_cache.find(skill_id);
    if (it != skill_cache.end())
    {
        return it->second;
    }

    return nullptr;
}

bool BattleSkillFactory::has_skill(uint16 skill_id) const
{
    std::lock_guard<std::mutex> lock(creators_mutex);
    return skill_creators.find(skill_id) != skill_creators.end();
}

void BattleSkillFactory::clear_cache()
{
    std::lock_guard<std::mutex> lock(cache_mutex);
    skill_cache.clear();
}

void BattleSkillFactory::register_all_skills()
{
    register_weapon_skills();
    register_magic_skills();
    register_misc_skills();
    register_passive_skills();
}

void BattleSkillFactory::register_weapon_skills()
{
    register_skills<BashSkill>({SM_BASH, MS_BASH});
}

// Register magic skills
void BattleSkillFactory::register_magic_skills()
{
    // MG_FIREBOLT, MG_COLDBOLT etc
}

// Register misc skills
void BattleSkillFactory::register_misc_skills()
{
    // NJ_ZENYNAGE, MO_EXTREMITYFIST etc
}

// Register passive skills
void BattleSkillFactory::register_passive_skills()
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
