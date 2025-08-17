// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "battle_skill.hpp"
#include <common/cbasetypes.hpp>
#include "../battle.hpp"
#include "../status.hpp"
#include "../pc.hpp"
#include "../map.hpp"

Skill::Skill(e_skill skill_id)
{
	this->skill_id_ = skill_id;
}

e_skill Skill::getSkillId() const
{
    return skill_id_;
}

void Skill::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const
{
    // no-op
}

void Skill::modifyHitRate(int16 &hit_rate, const block_list *src, const block_list *target, uint16 skill_lv) const
{
    // no-op
}

void Skill::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const
{
    // no-op
}
