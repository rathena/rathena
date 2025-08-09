#pragma once

#include "battle_skill.hpp"

class WeaponSkill : public BattleSkill
{
public:
    explicit WeaponSkill(e_skill skill_id) : BattleSkill(skill_id) {};

    e_battle_flag getAttackType() const override { return BF_WEAPON; }
    bool isWeaponBased() const override { return true; }
    bool isMagicBased() const override { return false; }

    // Weapon skills typically can critical and use weapon elements
    bool canCrit() const override { return true; }
    int32 getWeaponElement(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int16 weapon_position, bool calc_for_damage_only) const override;
};

/**
 * Base class for magic-based skills
 */
class MagicSkill : public BattleSkill
{
public:
    explicit MagicSkill(e_skill skill_id) : BattleSkill(skill_id) {};

    e_battle_flag getAttackType() const override { return BF_MAGIC; }
    bool isWeaponBased() const override { return false; }
    bool isMagicBased() const override { return true; }

    // Magic skills typically cannot critical and ignore weapon elements
    bool canCrit() const override { return false; }
    int32 getMagicElement(const block_list *src, const block_list *target, uint16 skill_lv, int32 mflag) const override;
};

/**
 * Base class for misc-type skills
 */
class MiscSkill : public BattleSkill
{
public:
    explicit MiscSkill(e_skill skill_id) : BattleSkill(skill_id) {};

    e_battle_flag getAttackType() const override { return BF_MISC; }
    bool isWeaponBased() const override { return false; }
    bool isMagicBased() const override { return false; }
    bool isMiscBased() const override { return true; }

    int32 getMiscElement(const block_list *src, const block_list *target, uint16 skill_lv, int32 mflag) const override;
};

/**
 * Base class for passive skills
 */
class PassiveSkill : public BattleSkill
{
public:
    explicit PassiveSkill(e_skill skill_id) : BattleSkill(skill_id) {};

    bool isOffensive() const override { return false; }
    bool canMiss() const override { return false; }
    void execute(const block_list *src, block_list *target, uint16 skill_lv, t_tick tick) const override {}
};

/**
 * Base class for ranged weapon skills
 */
class RangedWeaponSkill : public WeaponSkill
{
public:
    explicit RangedWeaponSkill(e_skill skill_id) : WeaponSkill(skill_id) {};

    bool usesAmmo() const override { return true; }
    int32 getRangeType(const block_list *src, const block_list *target, uint16 skill_lv) const override { return BF_LONG; }
};
