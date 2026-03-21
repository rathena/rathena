// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

// BO_ACIDIFIED_ZONE_FIRE
class SkillAcidifiedZoneFire : public SkillImplRecursiveDamageSplash {
public:
	SkillAcidifiedZoneFire();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
	void castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const override;
	void splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};


// BO_ACIDIFIED_ZONE_FIRE_ATK
class SkillActifiedZoneFireAttack : public WeaponSkillImpl {
public:
	SkillActifiedZoneFireAttack();

	void calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const override;
};
