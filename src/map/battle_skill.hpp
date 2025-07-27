#ifndef BATTLE_SKILL_HPP
#define BATTLE_SKILL_HPP

#include <memory>
#include "battle.hpp"
#include "status.hpp"

// Forward declarations

class BattleSkill {
public:
    virtual ~BattleSkill() = default;

    explicit BattleSkill(e_skill skill_id) : skill_id_(static_cast<uint16_t>(skill_id)) {}

    virtual uint16_t get_skill_id() const = 0;
    virtual const char* get_skill_name() const = 0;
    virtual e_battle_flag get_attack_type() const = 0; // BF_WEAPON, BF_MAGIC, BF_MISC
    
    virtual bool can_use(const block_list& source, const block_list& target) const;
    virtual bool can_hit_target(const block_list& source, const block_list& target) const;
    
    #pragma region MAJOR CALCULATION METHODS
    /**
     * Calculate base skill damage - replaces battle_calc_skill_base_damage() switch
     * Handles special base damage calculations per skill
     */
    virtual void calculate_base_damage(Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    /**
     * Calculate skill damage ratio - replaces battle_calc_attack_skill_ratio() switch  
     * Returns percentage modifier (100 = normal damage, 200 = double damage, etc.)
     */
    virtual int32 calculate_skill_ratio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32 base_skillratio) const;
    
    /**
     * Calculate constant damage addition - replaces battle_calc_skill_constant_addition() switch
     * Returns flat damage to add after percentage calculations
     */
    virtual int64 calculate_constant_addition(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    #pragma endregion

    #pragma region HIT/MISS CALCULATIONS
    
    /**
     * Modify hit rate for this skill - replaces hit rate switch statements
     */
    virtual void modify_hit_rate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    /**
     * Check if this skill ignores defense - replaces attack_ignores_def() switch cases
     */
    virtual bool ignores_defense(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int16 weapon_position) const;
    
    /**
     * Check if this attack is piercing - replaces is_attack_piercing() switch cases
     */
    virtual bool is_piercing_attack(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int16 weapon_position) const;
    
    /**
     * Check for critical hit modifications
     */
    virtual bool modify_critical_check(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv) const;

    #pragma endregion
    
    #pragma region ELEMENT CALCULATIONS
    /**
     * Get weapon element for this skill - replaces battle_get_weapon_element() switch cases
     */
    virtual int32 get_weapon_element(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int16 weapon_position, bool calc_for_damage_only) const;
    
    /**
     * Get magic element for this skill - replaces battle_get_magic_element() switch cases  
     */
    virtual int32 get_magic_element(const block_list* src, const block_list* target, uint16 skill_lv, int32 mflag) const;
    
    /**
     * Get misc element for this skill - replaces battle_get_misc_element() switch cases
     */
    virtual int32 get_misc_element(const block_list* src, const block_list* target, uint16 skill_lv, int32 mflag) const;
    
    #pragma endregion
    
    #pragma region RANGE AND TARGETING

    /**
     * Get range type (BF_SHORT/BF_LONG) - replaces battle_range_type() switch cases
     */
    virtual int32 get_range_type(const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    /**
     * Get skill damage properties - replaces battle_skill_get_damage_properties() logic
     */
    virtual std::bitset<NK_MAX> get_damage_properties(int32 is_splash) const;
    
    // ========== MULTI-HIT AND SPECIAL ATTACKS ==========
    
    /**
     * Modify multi-attack properties - replaces battle_calc_multi_attack() switch cases
     */
    virtual void modify_multi_attack(Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    /**
     * Apply div fix - replaces battle_apply_div_fix() switch cases
     */
    virtual void apply_div_fix(Damage* wd, uint16 skill_lv) const;
    
    #pragma endregion
    
    #pragma region STATUS EFFECTS AND BONUSES 
    /**
     * Apply status change bonuses - replaces battle_attack_sc_bonus() switch cases
     */
    virtual void apply_sc_bonus(Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    /**
     * Apply additional effects after damage - called from skill_additional_effect
     */
    virtual void apply_additional_effects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const;
    
    /**
     * Apply counter additional effects
     */
    virtual void apply_counter_effects(const block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type) const;
    
    #pragma endregion
    
    #pragma region MASTERIES AND EQUIPMENT 
    
    /**
     * Apply weapon masteries - replaces battle_calc_attack_masteries() switch cases
     */
    virtual void apply_masteries(Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    /**
     * Apply equipment bonuses - handles equipment-specific modifications
     */
    virtual void apply_equipment_modifiers(Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    #pragma endregion
        
    /**
     * Apply final attack modifiers - replaces battle_calc_weapon_final_atk_modifiers() switch cases
     */
    virtual void apply_final_modifiers(Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    /**
     * Handle post-defense calculations - replaces battle_calc_attack_post_defense() switch cases
     */
    virtual void apply_post_defense_modifiers(Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv) const;
    
    /**
     * Execute the complete skill calculation flow
     */
    virtual void execute(const block_list* src, block_list* target, uint16 skill_lv, t_tick tick) const;
    
    // ========== UTILITY METHODS ==========
    
    virtual bool is_offensive() const { return true; }
    virtual bool is_magic_based() const { return get_attack_type() == BF_MAGIC; }
    virtual bool is_weapon_based() const { return get_attack_type() == BF_WEAPON; }
    virtual bool is_misc_based() const { return get_attack_type() == BF_MISC; }
    virtual bool uses_ammo() const { return false; }
    virtual bool is_ground_skill() const { return false; }
    virtual bool can_crit() const { return is_weapon_based(); }
    virtual bool can_miss() const { return true; }
    
protected:
    // Helper methods for common calculations
    static map_session_data* get_player_data(const block_list* bl);
    static status_data* get_status_data(const block_list* bl);
    static status_change* get_status_change(const block_list* bl);
    static bool is_player(const block_list* bl);
    static bool is_mob(const block_list* bl);

    uint16_t skill_id_;
};

/**
 * Base class for weapon-based skills
 */
class WeaponSkill : public BattleSkill {
public:
    e_battle_flag get_attack_type() const override { return BF_WEAPON; }
    bool is_weapon_based() const override { return true; }
    bool is_magic_based() const override { return false; }
    
    // Weapon skills typically can critical and use weapon elements
    bool can_crit() const override { return true; }
    int32 get_weapon_element(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int16 weapon_position, bool calc_for_damage_only) const override;
};

/**
 * Base class for magic-based skills  
 */
class MagicSkill : public BattleSkill {
public:
    e_battle_flag get_attack_type() const override { return BF_MAGIC; }
    bool is_weapon_based() const override { return false; }
    bool is_magic_based() const override { return true; }
    
    // Magic skills typically cannot critical and ignore weapon elements
    bool can_crit() const override { return false; }
    int32 get_magic_element(const block_list* src, const block_list* target, uint16 skill_lv, int32 mflag) const override;
};

/**
 * Base class for misc-type skills
 */
class MiscSkill : public BattleSkill {
public:
    e_battle_flag get_attack_type() const override { return BF_MISC; }
    bool is_weapon_based() const override { return false; }
    bool is_magic_based() const override { return false; }
    bool is_misc_based() const override { return true; }
    
    int32 get_misc_element(const block_list* src, const block_list* target, uint16 skill_lv, int32 mflag) const override;
};

/**
 * Base class for passive skills
 */
class PassiveSkill : public BattleSkill {
public:
    bool is_offensive() const override { return false; }
    bool can_miss() const override { return false; }
    void execute(const block_list* src, block_list* target, uint16 skill_lv, t_tick tick) const override {}
};

/**
 * Base class for ranged weapon skills
 */
class RangedWeaponSkill : public WeaponSkill {
public:
    bool uses_ammo() const override { return true; }
    int32 get_range_type(const block_list* src, const block_list* target, uint16 skill_lv) const override { return BF_LONG; }
};

#endif // BATTLE_SKILL_HPP