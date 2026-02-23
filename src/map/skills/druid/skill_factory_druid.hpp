// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_factory.hpp"

#include "map/map.hpp"
#include "map/status.hpp"
#include "map/skill.hpp"

constexpr int32 kClawChainDuration = 5000;

int32 apply_splash_outer_sub(block_list* bl, va_list ap);

class SkillFactoryDruid : public SkillFactory {
public:
	virtual std::unique_ptr<const SkillImpl> create(const e_skill skill_id) const override;

	static void addThunderingCharge(block_list* src, uint16 skill_id, uint16 skill_lv, int32 charge);
	static void try_gain_growth_stacks(block_list* src, t_tick tick, e_skill skill_id);
	static e_skill resolve_quill_spear_skill(const status_change* sc, e_skill skill_id);
	static int32 get_madness_stage(const status_change* sc);
	static void try_gain_madness(block_list* src);
	static bool get_glacier_center_on_map(const block_list* src, const status_change* sc, int32& gx, int32& gy);
};
