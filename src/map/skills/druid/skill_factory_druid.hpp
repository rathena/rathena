// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_factory.hpp"

#include "map/map.hpp"
#include "map/status.hpp"
#include "map/skill.hpp"

namespace {
	void try_gain_growth_stacks(block_list* src, t_tick tick, e_skill skill_id);
	void try_gain_thundering_charge(block_list* src, const status_change* sc, e_skill skill_id, int32 gain);
	e_skill resolve_thundering_charge_skill(const status_change* sc, e_skill skill_id);
	e_skill resolve_quill_spear_skill(const status_change* sc, e_skill skill_id);
	int32 get_madness_stage(const status_change* sc);
	int32 apply_splash_outer_sub(block_list* bl, va_list ap);
	void try_gain_madness(block_list* src);
}

class SkillFactoryDruid : public SkillFactory {
public:
	virtual std::unique_ptr<const SkillImpl> create(const e_skill skill_id) const override;
};
