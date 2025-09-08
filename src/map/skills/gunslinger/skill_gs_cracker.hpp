#pragma once

#include "../weapon_skill_impl.hpp"

#include "../../battle.hpp"

class SkillGS_CRACKER : public WeaponSkillImpl {
public:
	SkillGS_CRACKER();

	void castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const override;
};