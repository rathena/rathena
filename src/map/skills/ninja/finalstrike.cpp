// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "finalstrike.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillFinalStrike::SkillFinalStrike() : WeaponSkillImpl(NJ_ISSEN) {
}

void SkillFinalStrike::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	int16 x, y;
	int16 dir = map_calc_dir(src, target->x, target->y);

	int16 i = 2; // Move 2 cells (From target)

	if (dir > 0 && dir < 4)
		x = -i;
	else if (dir > 4)
		x = i;
	else
		x = 0;
	if (dir > 2 && dir < 6)
		y = -i;
	else if (dir == 7 || dir < 2)
		y = i;
	else
		y = 0;

#ifdef RENEWAL
	// Doesn't have slide effect in GVG
	if (skill_check_unit_movepos(5, src, target->x + x, target->y + y, 1, 1)) {
		clif_blown(src);
		clif_spiritball(src);
	}
	skill_attack(BF_MISC, src, src, target, getSkillId(), skill_lv, tick, flag);
	status_set_hp(src, umax(status_get_max_hp(src) / 100, 1), 0);
	status_change_end(src, SC_NEN);
	status_change_end(src, SC_HIDING);
#else
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);

	status_set_hp(src, 1, 0);
	status_change_end(src, SC_NEN);
	status_change_end(src, SC_HIDING);

	// Doesn't have slide effect in GVG
	if (skill_check_unit_movepos(5, src, target->x + x, target->y + y, 1, 1)) {
		clif_blown(src);
		clif_spiritball(src);
	}
#endif
}
