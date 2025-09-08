#pragma once

#include "../skill_impl.hpp"

#include "../../battle.hpp"

class SkillNJ_HYOUSYOURAKU : public SkillImpl {
public:
	SkillNJ_HYOUSYOURAKU();
	
	void applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const override;
};