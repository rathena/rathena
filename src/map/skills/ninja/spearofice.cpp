// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "spearofice.hpp"

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/unit.hpp"

SkillSpearOfIce::SkillSpearOfIce() : SkillImpl(NJ_HYOUSENSOU) {
}

void SkillSpearOfIce::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	clif_skill_nodamage(src, *target, this->skill_id_, skill_lv);
	skill_attack(BF_MAGIC, src, src, target, this->skill_id_, skill_lv, tick, flag);
}
