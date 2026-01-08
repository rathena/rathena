// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ragingfiredragon.hpp"

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/unit.hpp"

SkillRagingFireDragon::SkillRagingFireDragon() : SkillImpl(NJ_BAKUENRYU) {
}

void SkillRagingFireDragon::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	clif_skill_nodamage(src, *target, this->skill_id_, skill_lv);
	skill_unitsetting(src, this->skill_id_, skill_lv, target->x, target->y, 0);
}
