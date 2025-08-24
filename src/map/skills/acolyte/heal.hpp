#pragma once

#include "../skill_impl.hpp"

#include "../../battle.hpp"

class SkillAL_HEAL : public SkillImpl {
public:
	SkillAL_HEAL();
	
	void castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const override;
	void castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};