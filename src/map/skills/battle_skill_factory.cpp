#include <common/showmsg.hpp>

#include "battle_skill_factory.hpp"
#include "./swordsman/swordsman.hpp"
#include "./mercenary/mercenary.hpp"

void BattleSkillFactory::registerSkill(uint16 skill_id, const std::shared_ptr<BattleSkill> &skill)
{
    std::shared_ptr<s_skill_db> skill_entry = skill_db.find(skill_id);
    if (skill_entry)
    {
        skill_entry->impl = skill;
    }
    else
    {
        ShowError("registerSkill: could not find s_skill_db with %d", skill_id);
    }
}

void BattleSkillFactory::registerAllSkills()
{
    Swordsman::register_skills();
    Mercenary::register_skills();
}
