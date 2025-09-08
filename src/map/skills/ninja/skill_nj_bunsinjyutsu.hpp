#pragma once

#include "../skill_impl.hpp"

#include "../../battle.hpp"

class SkillNJ_BUNSINJYUTSU : public SkillImpl {
public:
	SkillNJ_BUNSINJYUTSU();
	
	int32 castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const override;
};