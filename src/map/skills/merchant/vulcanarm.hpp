// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_impl.hpp"

class SkillVulcanArm : public SkillImplRecursiveDamageSplash {
public:
	SkillVulcanArm();

	void modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const override;
	void calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const override;
};
