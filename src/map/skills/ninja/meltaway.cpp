// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "meltaway.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillMeltAway::SkillMeltAway() : SkillImpl(SS_TOKEDASU) {
}

void SkillMeltAway::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 700 * skill_lv;
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}

void SkillMeltAway::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillMeltAway::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea(skill_area_sub, src->m, x - i, y - i, x + i, y + i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	unit_setdir(src, map_calc_dir_xy(src->x, src->y, x, y, unit_getdir(src)));
	skill_blown(src, src, skill_get_blewcount(getSkillId(), skill_lv), unit_getdir(src), (enum e_skill_blown)(BLOWN_IGNORE_NO_KNOCKBACK | BLOWN_DONT_SEND_PACKET));
	clif_blown(src);
}
