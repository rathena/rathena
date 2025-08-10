#pragma once

#include "battle_skill.hpp"

class WeaponSkill : public BattleSkill
{
public:
    explicit WeaponSkill(e_skill skill_id) : BattleSkill(skill_id) {};

    e_battle_flag getAttackType() const override { return BF_WEAPON; }
};

/**
 * Base class for magic-based skills
 */
class MagicSkill : public BattleSkill
{
public:
    explicit MagicSkill(e_skill skill_id) : BattleSkill(skill_id) {};

    e_battle_flag getAttackType() const override { return BF_MAGIC; }
};

/**
 * Base class for misc-type skills
 */
class MiscSkill : public BattleSkill
{
public:
    explicit MiscSkill(e_skill skill_id) : BattleSkill(skill_id) {};

    e_battle_flag getAttackType() const override { return BF_MISC; }
};

/**
 * Base class for passive skills
 */
class PassiveSkill : public BattleSkill
{
public:
    explicit PassiveSkill(e_skill skill_id) : BattleSkill(skill_id) {};
};

/**
 * Base class for ranged weapon skills
 */
class RangedWeaponSkill : public WeaponSkill
{
public:
    explicit RangedWeaponSkill(e_skill skill_id) : WeaponSkill(skill_id) {};
};
