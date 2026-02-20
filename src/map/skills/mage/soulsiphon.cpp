// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soulsiphon.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSoulSiphon::SkillSoulSiphon() : SkillImpl(PF_SOULBURN) {
}

void SkillSoulSiphon::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (rnd() % 100 < (skill_lv < 5 ? 30 + skill_lv * 10 : 70)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		if (skill_lv == 5) {
			skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
		}
		status_percent_damage(src, target, 0, 100, false);
	} else {
		clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
		if (skill_lv == 5) {
			skill_attack(BF_MAGIC, src, src, src, getSkillId(), skill_lv, tick, flag);
		}
		status_percent_damage(src, src, 0, 100, false);
	}
}
