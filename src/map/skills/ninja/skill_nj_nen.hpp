#pragma once

#include "../skill_impl.hpp"

#include "../../battle.hpp"

class SkillNJ_NEN : public SkillImpl {
public:
	SkillNJ_NEN();
	
	int32 castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const override;
};