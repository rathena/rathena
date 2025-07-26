#ifndef BATTLE_SKILL_CATEGORIES_HPP
#define BATTLE_SKILL_CATEGORIES_HPP

#include "battle_skill.hpp"

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

#endif // BATTLE_SKILL_FACTORY_HPP