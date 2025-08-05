#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <vector>
#include <common/cbasetypes.hpp>
#include "battle_skill.hpp"
#include "skill.hpp"

/**
 * Factory class for creating and managing BattleSkill instances
 * Uses singleton pattern with thread-safe skill registration and caching
 */
class BattleSkillFactory
{
public:
    // Destructor
    ~BattleSkillFactory() = default;

    static void registerSkill(uint16 skill_id, const std::shared_ptr<BattleSkill>& skill)
    {
        auto skill_entry = skill_db.find(skill_id);
        if (skill_entry) {
            skill_entry->impl = skill;
        }
    }

    static void registerAllSkills();

private:
    static void registerWeaponSkills();
    static void registerMagicSkills();
    static void registerMiscSkills();
    static void registerPassiveSkills();
};
