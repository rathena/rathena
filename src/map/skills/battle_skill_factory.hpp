#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <vector>
#include <common/cbasetypes.hpp>
#include "battle_skill.hpp"
#include "../skill.hpp"

/**
 * Factory class for creating and managing BattleSkill instances
 * Uses singleton pattern with thread-safe skill registration and caching
 */
class BattleSkillFactory
{
public:
    // Destructor
    ~BattleSkillFactory() = default;

    static void registerSkill(uint16 skill_id, const std::shared_ptr<BattleSkill>& skill);
    static void registerAllSkills();
};
