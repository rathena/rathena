// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "infiltrate.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillInfiltrate::SkillInfiltrate() : SkillImpl(SS_SHIMIRU) {
}

void SkillInfiltrate::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 700 * skill_lv;
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}

void SkillInfiltrate::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	struct unit_data *ud = unit_bl2ud(src);

	if (!check_distance_bl(src, target, 0)) {
		uint8 dir = map_calc_dir(src, target->x, target->y);
		int16 x, y;

		if (dir > DIR_NORTH && dir < DIR_SOUTH)
			x = -1;
		else if (dir > DIR_SOUTH)
			x = 1;
		else
			x = 0;

		if (dir > DIR_WEST && dir < DIR_EAST)
			y = -1;
		else if (dir == DIR_NORTHEAST || dir < DIR_WEST)
			y = 1;
		else
			y = 0;

		if (battle_check_target(src, target, BCT_ENEMY) > 0 && unit_movepos(src, target->x + x, target->y + y, 2, true)) {// Display movement + animation.
			dir = dir < 4 ? dir+4 : dir-4; // change direction [Celest]
			unit_setdir(target,dir);
			clif_blown(src);
		} else {
			if (sd != nullptr) {
				clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_TARGET_SHADOW_SPACE );
			}
			return;
		}
	}

	if (ud == nullptr)
		return;

	for (const std::shared_ptr<s_skill_unit_group>& sug : ud->skillunits) {
		skill_unit* su = sug->unit;
		std::shared_ptr<s_skill_unit_group> sg = su->group;
		int16 dx = src->x - su->x;
		int16 dy = src->y - su->y;

		for( size_t count = 0; count < 1000; count++ ){
			if (map_foreachincell(skill_shimiru_check_cell, src->m, su->x + dx, su->y + dy, BL_CHAR|BL_SKILL) == 0)
				break;
			dx += rnd() % 3 - 1;
			dy += rnd() % 3 - 1;
		}

		if (sug->skill_id == SS_SHINKIROU)
			skill_unit_move_unit_group(sg, src->m, dx,dy);
	}

	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);

	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}
