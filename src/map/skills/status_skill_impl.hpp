// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include "skill_impl.hpp"

class StatusSkillImpl : public SkillImpl
{
private:
	bool end_if_running;

public:
	StatusSkillImpl(e_skill skillId, bool end_if_running = false);

	virtual void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const override;
};
