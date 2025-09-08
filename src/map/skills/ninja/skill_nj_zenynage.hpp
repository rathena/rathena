#pragma once

#include "../skill_impl.hpp"

#include "../../battle.hpp"

class SkillNJ_ZENYNAGE : public SkillImpl {
public:
	SkillNJ_ZENYNAGE();
	
	void castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};