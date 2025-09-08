#pragma once

#include "../weapon_skill_impl.hpp"

#include "../../battle.hpp"

class SkillMC_CARTREVOLUTION : public WeaponSkillImpl {
public:
	SkillMC_CARTREVOLUTION();
	
	// Method implementations
	void calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const override;
	void modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const override;
	void castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};