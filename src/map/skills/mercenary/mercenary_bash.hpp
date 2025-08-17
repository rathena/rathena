#pragma once

#include "../../battle.hpp"
#include "../../pc.hpp"
#include "../../skill.hpp"

class SkillMercenaryBash : public Skill
{
public:
    SkillMercenaryBash() : Skill(MS_BASH) {}

    void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const override
    {
        // Base 100% + 30% per level
        base_skillratio += 30 * skill_lv;
    }

    void modifyHitRate(int16 &hit_rate, const block_list *src, const block_list *target, uint16 skill_lv) const override
    {
        // +5% hit per level
        hit_rate += hit_rate * 5 * skill_lv / 100;
    }
};
