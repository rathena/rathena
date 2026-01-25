// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_factory.hpp"

namespace {
	void try_gain_growth_stacks(block_list* src, t_tick tick, e_skill skill_id);
}

class SkillFactoryDruid : public SkillFactory {
public:
	virtual std::unique_ptr<const SkillImpl> create(const e_skill skill_id) const override;
};
