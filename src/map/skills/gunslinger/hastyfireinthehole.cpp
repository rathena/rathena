// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hastyfireinthehole.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHastyFireInTheHole::SkillHastyFireInTheHole() : WeaponSkillImpl(NW_HASTY_FIRE_IN_THE_HOLE) {
}

void SkillHastyFireInTheHole::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	if (flag & 1){
		i++;
	}
	if (flag & 2){
		i++;
	}
	map_foreachinallarea(skill_area_sub,
		src->m, x - i, y - i, x + i, y + i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1,
		skill_castend_damage_id);
	if (!(flag & 1)) {
		skill_addtimerskill(src, tick + 300, 0, x, y, getSkillId(), skill_lv, 0, flag | 1 | SKILL_NOCONSUME_REQ);
		skill_addtimerskill(src, tick + 600, 0, x, y, getSkillId(), skill_lv, 0, flag | 3 | SKILL_NOCONSUME_REQ);
	}
}

void SkillHastyFireInTheHole::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1500 + 1500 * skill_lv;
	skillratio += pc_checkskill(sd, NW_GRENADE_MASTERY) * 20;
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}
