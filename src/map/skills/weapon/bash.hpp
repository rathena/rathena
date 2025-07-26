#ifndef BASH_SKILL_HPP
#define BASH_SKILL_HPP

#include "battle_skill_categories.hpp"
#include "battle.hpp"
#include "pc.hpp"

/**
 * BashSkill - Implementation of SM_BASH and MS_BASH
 * A basic melee attack skill that deals increased damage and has stun effect
 */
class BashSkill : public WeaponSkill {
public:
    // Core identification
    uint16 get_skill_id() const override { 
        return SM_BASH; // Note: MS_BASH shares same implementation
    }
    
    const char* get_skill_name() const override { 
        return "Bash"; 
    }

    // ========== REPLACES battle_calc_attack_skill_ratio() SWITCH ==========
    int32 calculate_skill_ratio(const Damage* wd, const block_list* src, 
                               const block_list* target, uint16 skill_lv, int32 base_skillratio) const override {
        return base_skillratio + 30 * skill_lv;  // Base 100% + 30% per level
    }

    // ========== REPLACES hit rate calculation SWITCH ==========
    void modify_hit_rate(int16& hit_rate, const block_list* src, 
                         const block_list* target, uint16 skill_lv) const override {
        hit_rate += hit_rate * 5 * skill_lv / 100;  // +5% hit per level
    }

    // ========== REPLACES skill_additional_effect() SWITCH ==========
    void apply_additional_effects(block_list* src, block_list* target, 
                                 uint16 skill_lv, t_tick tick, int32 attack_type, 
                                 enum damage_lv dmg_lv) const override {
        
        map_session_data* sd = get_player_data(src);
        
        // Stun effect only if skill level > 5 and has Fatal Blow
        if (sd && skill_lv > 5 && pc_checkskill(sd, SM_FATALBLOW) > 0) {
            int32 stun_chance = (skill_lv - 5) * sd->status.base_level * 10;
            status_change_start(src, target, SC_STUN, stun_chance, skill_lv, 0, 0, 0,
                              skill_get_time2(get_skill_id(), skill_lv), SCSTART_NONE);
        }
    }

    // ========== VALIDATION ==========
    bool can_use(const block_list& source, const block_list& target) const override {
        // Bash requires a melee weapon (not bare hands)
        if (source.type == BL_PC) {
            const map_session_data* sd = (const map_session_data*)&source;
            if (sd->status.weapon == W_FIST) {
                return false; // Cannot bash with bare hands
            }
            
            // Cannot use with ranged weapons
            switch (sd->status.weapon) {
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
        
        return WeaponSkill::can_use(source, target);
    }

    // ========== UTILITY METHODS ==========
    bool can_crit() const override { 
        return true; // Bash can critical hit
    }
    
    bool uses_ammo() const override { 
        return false; // Bash doesn't consume ammo
    }
    
    int32 get_range_type(const block_list* src, const block_list* target, 
                        uint16 skill_lv) const override {
        return BF_SHORT; // Always short range
    }
};

#endif // BASH_SKILL_HPP