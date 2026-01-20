// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include <config/core.hpp>

#include "../skill_impl.hpp"
#include "../status_skill_impl.hpp"

#ifdef RENEWAL
class SkillImpositioManus : public SkillImpl {
public:
	SkillImpositioManus();

	void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};
#else
class SkillImpositioManus : public StatusSkillImpl {
public:
	SkillImpositioManus();
};
#endif
