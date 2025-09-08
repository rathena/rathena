#pragma once

#include "../weapon_skill_impl.hpp"

#include "../../battle.hpp"

class SkillNJ_KASUMIKIRI : public WeaponSkillImpl {
public:
	SkillNJ_KASUMIKIRI();
	
	void calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const override;
	void castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
	int32 castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const override;
};