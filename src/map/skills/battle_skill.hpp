#pragma once

#include <memory>
#include "../battle.hpp"
#include "../status.hpp"

class Skill
{
public:
    explicit Skill(e_skill skill_id);
    virtual ~Skill() = default;

	e_skill getSkillId() const;

    /**
     * Calculate skill damage ratio - replaces battle_calc_attack_skill_ratio() switch
     */
    virtual void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const;

    /**
     * Modify hit rate for this skill - replaces hit rate switch statements
     */
    virtual void modifyHitRate(int16 &hit_rate, const block_list *src, const block_list *target, uint16 skill_lv) const;

    /**
     * Apply additional effects after damage - called from skill_additional_effect
     */
    virtual void applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const;

protected:
	e_skill skill_id_;
};
