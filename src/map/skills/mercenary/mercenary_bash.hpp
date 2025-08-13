#pragma once

#include "../battle_skill_categories.hpp"
#include "../../battle.hpp"
#include "../../pc.hpp"
#include "../../skill.hpp"

class MercenaryBashSkill : public WeaponSkill
{
public:
    MercenaryBashSkill() : WeaponSkill(MS_BASH) {}

    void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const override
    {
        base_skillratio += 30 * skill_lv; // Base 100% + 30% per level
    }

    void modifyHitRate(int16 &hit_rate, const block_list *src, const block_list *target, uint16 skill_lv) const override
    {
        hit_rate += hit_rate * 5 * skill_lv / 100; // +5% hit per level
    }
};
