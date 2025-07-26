#ifndef BATTLE_SKILL_FACTORY_HPP
#define BATTLE_SKILL_FACTORY_HPP

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
    // Skill creators - functions that create skill instances
    std::unordered_map<uint16, std::function<std::shared_ptr<BattleSkill>()>> skill_creators;

    // Skill cache - stores created instances for performance
    mutable std::unordered_map<uint16, std::shared_ptr<BattleSkill>> skill_cache;

    // Default skill for unimplemented skills
    mutable std::shared_ptr<BattleSkill> default_skill;

    // Thread safety
    mutable std::mutex cache_mutex;
    mutable std::mutex creators_mutex;

    // Private constructor for singleton
    BattleSkillFactory();

public:
    // Singleton access
    static BattleSkillFactory &instance();

    // Delete copy constructor and assignment operator
    BattleSkillFactory(const BattleSkillFactory &) = delete;
    BattleSkillFactory &operator=(const BattleSkillFactory &) = delete;

    // Destructor
    ~BattleSkillFactory() = default;

    /**
     * Register a skill type with template for type safety
     * @tparam SkillType The skill class type
     * @param skill_id The skill ID to register
     */
    template <typename SkillType>
    void register_skill(uint16 skill_id)
    {
        static_assert(std::is_base_of_v<BattleSkill, SkillType>,
                      "SkillType must inherit from BattleSkill");

        std::lock_guard<std::mutex> lock(creators_mutex);
        skill_creators[skill_id] = []() -> std::shared_ptr<BattleSkill>
        {
            return std::make_shared<SkillType>();
        };
    }

    /**
     * Register multiple skills of the same type
     * @tparam SkillType The skill class type
     * @param skill_ids Vector of skill IDs to register
     */
    template <typename SkillType>
    void register_skills(const std::vector<uint16> &skill_ids)
    {
        for (uint16 skill_id : skill_ids)
        {
            register_skill<SkillType>(skill_id);
        }
    }

    /**
     * Get a skill instance (cached)
     * @param skill_id The skill ID
     * @return Shared pointer to skill instance, or default skill if not found
     */
    std::shared_ptr<BattleSkill> get_skill(uint16 skill_id) const;

    /**
     * Check if a skill is registered
     * @param skill_id The skill ID
     * @return True if skill is registered
     */
    bool has_skill(uint16 skill_id) const;

    /**
     * Clear skill cache (useful for testing or memory management)
     */
    void clear_cache();

    /**
     * Register all skills in the game
     * This is called during server initialization
     */
    void register_all_skills();

private:
    void register_weapon_skills();
    void register_magic_skills();
    void register_misc_skills();
    void register_passive_skills();
};

#endif // BATTLE_SKILL_FACTORY_HPP