#pragma once

#include "../weapon_skill_impl.hpp"

#include "../../battle.hpp"

class SkillGS_CHAINACTION : public WeaponSkillImpl {
public:
	SkillGS_CHAINACTION();

	void castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};