#pragma once

#include <memory>
#include "../battle.hpp"
#include "../status.hpp"

class BattleSkill
{
public:
    virtual ~BattleSkill() = default;

    explicit BattleSkill(e_skill skill_id) : skill_id_(static_cast<uint16_t>(skill_id)) {}

    virtual uint16_t getSkillId() const = 0;
    virtual const char *getSkillName() const = 0;
    virtual e_battle_flag getAttackType() const = 0; // BF_WEAPON, BF_MAGIC, BF_MISC

    /**
     * Calculate skill damage ratio - replaces battle_calc_attack_skill_ratio() switch
     * Returns percentage modifier (100 = normal damage, 200 = double damage, etc.)
     */
    virtual int32 calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 base_skillratio) const;

    /**
     * Modify hit rate for this skill - replaces hit rate switch statements
     */
    virtual void modifyHitRate(int16 &hit_rate, const block_list *src, const block_list *target, uint16 skill_lv) const;

    /**
     * Apply additional effects after damage - called from skill_additional_effect
     */
    virtual void applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const;

protected:
    // Helper methods for common calculations
    static map_session_data *getPlayerData(const block_list *bl);
    static status_data *getStatusData(const block_list *bl);
    static status_change *getStatusChange(const block_list *bl);

    uint16_t skill_id_;
};
