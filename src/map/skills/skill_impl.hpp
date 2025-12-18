// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../battle.hpp"

class SkillImpl {
public:
	explicit SkillImpl(e_skill skill_id);
	virtual ~SkillImpl() = default;

	e_skill getSkillId() const;

	/**
	 * Effect of the skill - replaces switch statements in skill_castend_nodamage_id
	 */
	virtual void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const;

	/**
	 * Effect of the skill - replaces switch statements in skill_castend_damage_id
	 */
	virtual void castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const;

	/**
	 * Effect of the skill - replaces switch statements in skill_castend_pos2
	 */
	virtual void castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const;

	/**
	 * Calculate skill damage ratio - replaces battle_calc_attack_skill_ratio() switch
	 */
	virtual void calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const;

	/**
	 * Modify hit rate for this skill - replaces hit rate switch statements
	 */
	virtual void modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const;

	/**
	 * Apply additional effects after damage - called from skill_additional_effect
	 */
	virtual void applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const;

protected:
	e_skill skill_id_;
};

class SkillImplRecursiveDamageSplash : public SkillImpl{
public:
	explicit SkillImplRecursiveDamageSplash(e_skill skill_id);

	virtual void castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const override;

	virtual void castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const override;

	virtual int16 getSearchSize(uint16 skill_lv) const;

	virtual int16 getSplashSearchSize(uint16 skill_lv) const;

	virtual int32 getSplashTarget(block_list* src) const;

	virtual void splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const;

	virtual int64 splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const;
};
