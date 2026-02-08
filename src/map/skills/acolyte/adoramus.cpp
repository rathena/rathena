// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "adoramus.hpp"

#include <config/const.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillAdoramus::SkillAdoramus() : SkillImplRecursiveDamageSplash(AB_ADORAMUS) {
}

void SkillAdoramus::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	sc_start(src,target, SC_ADORAMUS, skill_lv * 4 + (sd ? sd->status.job_level : 50) / 2, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillAdoramus::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += - 100 + 300 + 250 * skill_lv;
	RE_LVL_DMOD(100);
}

int64 SkillAdoramus::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	if (map_getcell(target->m, target->x, target->y, CELL_CHKLANDPROTECTOR))
		return 0; // No damage should happen if the target is on Land Protector

	return SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);
}
