#pragma once

#include "../skill_impl.hpp"

#include "../../battle.hpp"

class SkillGS_FLING : public SkillImpl {
public:
	SkillGS_FLING();

	void castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
	void applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const override;
};