#include "battle_skill_factory.hpp"

#include <common/showmsg.hpp>
#include "./swordsman/swordsman.hpp"
#include "./mercenary/mercenary.hpp"

void BattleSkillFactory::registerSkill(const e_skill skill_id, const std::shared_ptr<Skill> &skill)
{
    std::shared_ptr<s_skill_db> skill_entry = skill_db.find(skill_id);
    if (skill_entry != nullptr)
    {
        skill_entry->impl = skill;
    }
    else
    {
        ShowError("registerSkill: skill ID %hu not found in skill DB", skill_id);
    }
}

void BattleSkillFactory::registerAllSkills()
{
    Swordsman::register_skills();
    Mercenary::register_skills();
}
