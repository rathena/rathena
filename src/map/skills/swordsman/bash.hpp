#pragma once

#include "../../battle.hpp"
#include "../../pc.hpp"
#include "../../skill.hpp"

#include "../battle_skill.hpp"

class SkillBash : public Skill
{
public:
	SkillBash();

	void calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const override;
	void modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const override;
	void applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const override;
};
