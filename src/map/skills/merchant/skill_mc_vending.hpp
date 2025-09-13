#pragma once

#include "../skill_impl.hpp"

#include "../../battle.hpp"

class SkillMC_VENDING : public SkillImpl {
public:
	SkillMC_VENDING();
	
	// Method implementations
	int32 castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const override;
};