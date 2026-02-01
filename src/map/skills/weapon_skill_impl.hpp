// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "skill_impl.hpp"

class WeaponSkillImpl : public SkillImpl {
public:
	WeaponSkillImpl(e_skill skill_id);

	virtual void castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};
