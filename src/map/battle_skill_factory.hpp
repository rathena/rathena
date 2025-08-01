#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <vector>
#include <common/cbasetypes.hpp>
#include "battle_skill.hpp"

/**
 * Factory class for creating and managing BattleSkill instances
 * Uses singleton pattern with thread-safe skill registration and caching
 */
class BattleSkillFactory
{
private:
    std::unordered_map<uint16, std::shared_ptr<BattleSkill>> skill_db;
    BattleSkillFactory();

public:
    // Singleton access
    static BattleSkillFactory &instance();

    // Delete copy constructor and assignment operator
    BattleSkillFactory(const BattleSkillFactory &) = delete;
    BattleSkillFactory &operator=(const BattleSkillFactory &) = delete;

    // Destructor
    ~BattleSkillFactory() = default;

    void registerSkill(uint16 skill_id, const std::shared_ptr<BattleSkill>& skill)
    {
        skill_db[skill_id] = skill;
    }

    /**
     * Get a skill instance (cached)
     * @param skill_id The skill ID
     * @return Shared pointer to skill instance, or default skill if not found
     */
    std::shared_ptr<BattleSkill> getSkill(uint16 skill_id) const;

    /**
     * Check if a skill is registered
     * @param skill_id The skill ID
     * @return True if skill is registered
     */
    bool hasSkill(uint16 skill_id) const;

    /**
     * Register all skills in the game
     * This is called during server initialization
     */
    void registerAllSkills();

private:
    void registerWeaponSkills();
    void registerMagicSkills();
    void registerMiscSkills();
    void registerPassiveSkills();
};
