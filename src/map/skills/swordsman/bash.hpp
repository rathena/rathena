#pragma once

#include "../battle_skill_categories.hpp"
#include "../../battle.hpp"
#include "../../pc.hpp"
#include "../../skill.hpp"

class BashSkill : public WeaponSkill
{
public:
    BashSkill() : WeaponSkill(SM_BASH) {}

    int32 calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 base_skillratio) const override
    {
        return base_skillratio + 30 * skill_lv; // Base 100% + 30% per level
    }

    void modifyHitRate(int16 &hit_rate, const block_list *src, const block_list *target, uint16 skill_lv) const override
    {
        hit_rate += hit_rate * 5 * skill_lv / 100; // +5% hit per level
    }

    void applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const override
    {
        map_session_data *sd = nullptr;

        if (src && src->type == BL_PC)
        {
            sd = (map_session_data *)src;
        }

        // Stun effect only if skill level > 5 and has Fatal Blow
        if (sd && skill_lv > 5 && pc_checkskill(sd, SM_FATALBLOW) > 0)
        {
            int32 stun_chance = (skill_lv - 5) * sd->status.base_level * 10;
            status_change_start(src, target, SC_STUN, stun_chance, skill_lv, 0, 0, 0, skill_get_time2(getSkillId(), skill_lv), SCSTART_NONE);
        }
    }
};
