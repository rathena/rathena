#pragma once

#include "../skill_impl.hpp"

#include "../../battle.hpp"

class SkillAL_HOLYWATER : public SkillImpl {
public:
	SkillAL_HOLYWATER();
	
	void castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const override;
};