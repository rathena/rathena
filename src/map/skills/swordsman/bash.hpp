#pragma once

#include "../battle_skill_categories.hpp"
#include "../../battle.hpp"
#include "../../pc.hpp"
#include "../../skill.hpp"

class BashSkill : public WeaponSkill
{
public:
    BashSkill() : WeaponSkill(SM_BASH) {}
    
    // Core identification
    uint16_t getSkillId() const override
    {
        return skill_id_;
    }

    const char *getSkillName() const override
    {
        return skill_get_name(skill_id_);
    }

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
        map_session_data *sd = getPlayerData(src);

        // Stun effect only if skill level > 5 and has Fatal Blow
        if (sd && skill_lv > 5 && pc_checkskill(sd, SM_FATALBLOW) > 0)
        {
            int32 stun_chance = (skill_lv - 5) * sd->status.base_level * 10;
            status_change_start(src, target, SC_STUN, stun_chance, skill_lv, 0, 0, 0, skill_get_time2(getSkillId(), skill_lv), SCSTART_NONE);
        }
    }

    bool canUse(const block_list &source, const block_list &target) const override
    {
        // Bash requires a melee weapon (not bare hands)
        if (source.type == BL_PC)
        {
            const map_session_data *sd = (const map_session_data *)&source;
            if (sd->status.weapon == W_FIST)
            {
                return false; // Cannot bash with bare hands
            }

            // Cannot use with ranged weapons
            switch (sd->status.weapon)
            {
            case W_BOW:
            case W_MUSICAL:
            case W_WHIP:
            case W_REVOLVER:
            case W_RIFLE:
            case W_GATLING:
            case W_SHOTGUN:
            case W_GRENADE:
                return false;
            default:
                break;
            }
        }

        return WeaponSkill::canUse(source, target);
    }

    bool canCrit() const override
    {
        return true; // Bash can critical hit
    }

    bool usesAmmo() const override
    {
        return false; // Bash doesn't consume ammo
    }

    int32 getRangeType(const block_list *src, const block_list *target, uint16 skill_lv) const override
    {
        return BF_SHORT; // Always short range
    }
};