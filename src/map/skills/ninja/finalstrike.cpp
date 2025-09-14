// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "finalstrike.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "../../unit.hpp"

SkillFinalStrike::SkillFinalStrike() : SkillImpl(NJ_ISSEN) {
}

void SkillFinalStrike::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
#ifdef RENEWAL
	{
		int16 x, y;
		int16 dir = map_calc_dir(src, target->x, target->y);

		// Move 2 cells (From target)
		if (dir > 0 && dir < 4)
			x = -2;
		else if (dir > 4)
			x = 2;
		else
			x = 0;
		if (dir > 2 && dir < 6)
			y = -2;
		else if (dir == 7 || dir < 2)
			y = 2;
		else
			y = 0;
		// Doesn't have slide effect in GVG
		if (skill_check_unit_movepos(5, src, target->x + x, target->y + y, 1, 1)) {
			clif_blown(src);
			clif_spiritball(src);
		}
		skill_attack(BF_MISC, src, src, target, this->skill_id_, skill_lv, tick, flag);
		status_set_hp(src, umax(status_get_max_hp(src) / 100, 1), 0);
		status_change_end(src, SC_NEN);
		status_change_end(src, SC_HIDING);
	}
#else
	{
		struct block_list *mbl = target; // For NJ_ISSEN
		int16 x, y, i = 2; // Move 2 cells (From target)
		int16 dir = map_calc_dir(src, target->x, target->y);

		skill_attack(BF_WEAPON, src, src, target, this->skill_id_, skill_lv, tick, flag);
		status_set_hp(src, 1, 0);
		status_change_end(src, SC_NEN);
		status_change_end(src, SC_HIDING);

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
		// Ashura Strike still has slide effect in GVG
		if ((mbl == src || (!map_flag_gvg2(src->m) && !map_getmapflag(src->m, MF_BATTLEGROUND))) &&
		    unit_movepos(src, mbl->x + x, mbl->y + y, 1, 1)) {
			clif_blown(src);
			clif_spiritball(src);
		}
	}
#endif
}
