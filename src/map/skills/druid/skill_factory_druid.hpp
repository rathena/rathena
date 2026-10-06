// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "../skill_factory.hpp"

#include "map/status.hpp"
class SkillFactoryDruid : public SkillFactory {
public:
	virtual std::unique_ptr<const SkillImpl> create(const e_skill skill_id) const override;

	static void addThunderingCharge(block_list* src, uint16 skill_id, uint16 skill_lv, int32 charge);
	static e_skill resolve_quill_spear_skill(const status_change* sc, e_skill skill_id);
};
