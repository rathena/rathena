#pragma once

#include "../skill_impl.hpp"

#include "../../battle.hpp"

class SkillTK_READYSTORM : public SkillImpl {
public:
	SkillTK_READYSTORM();

	void castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const override;
};