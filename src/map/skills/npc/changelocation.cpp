// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "changelocation.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/status.hpp"

SkillChangeLocation::SkillChangeLocation() : SkillImpl(NPC_MOVE_COORDINATE) {
}

void SkillChangeLocation::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int16 px = target->x, py = target->y;
	if (!skill_check_unit_movepos(0, target, src->x, src->y, 1, 1)) {
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	clif_blown(target);

	// If caster is not a boss, switch coordinates with the target
	if (status_get_class_(src) != CLASS_BOSS) {
		if (!skill_check_unit_movepos(0, src, px, py, 1, 1)) {
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}

		clif_blown(src);
	}
}
