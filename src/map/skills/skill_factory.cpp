#include "skill_factory.hpp"

#include <common/showmsg.hpp>
#include "./swordsman/skill_factory_swordsman.hpp"
#include "./mercenary/skill_factory_mercenary.hpp"

void SkillFactory::registerSkill(const e_skill skill_id, const std::shared_ptr<Skill> &skill)
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

void SkillFactory::registerAllSkills()
{
    SkillFactorySwordsman::registerSkills();
    SkillFactoryMercenary::registerSkills();
}
