#pragma once

#include "../weapon_skill_impl.hpp"

#include "../../battle.hpp"

class SkillGS_ADJUSTMENT : public WeaponSkillImpl {
public:
	SkillGS_ADJUSTMENT();

	void castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const override;
};