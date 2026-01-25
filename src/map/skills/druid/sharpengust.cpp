// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sharpengust.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillSharpenGust::SkillSharpenGust() : SkillImplRecursiveDamageSplash(KR_SHARPEN_GUST) {
}

void SkillSharpenGust::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (flag & 1) {
		SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
		return;
	}

	const int16 path_width = 1;
	const int32 path_length = 7;
	const uint8 dir = map_calc_dir(src, target->x, target->y);
	const int16 dir_x = dirx[dir];
	const int16 dir_y = diry[dir];
	const int16 start_x = src->x;
	const int16 start_y = src->y;
	const int16 end_x = src->x + dir_x;
	const int16 end_y = src->y + dir_y;

	skill_area_temp[0] = 0;
	skill_area_temp[1] = target->id;
	skill_area_temp[2] = 0;

	if (battle_config.skill_eightpath_algorithm) {
		// Use official AoE algorithm
		map_foreachindir(skill_area_sub, src->m, start_x, start_y, end_x, end_y, path_width, path_length, 1, splash_target(src),
			src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | SD_SPLASH | 1, skill_castend_damage_id);
	}
	else {
		map_foreachindir(skill_area_sub, src->m, start_x, start_y, end_x, end_y, path_width, path_length, 1, splash_target(src),
			src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | SD_SPLASH | 1, skill_castend_damage_id);
	}
}

void SkillSharpenGust::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 1000 + 80 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_ENRAGE_RAPTOR)) {
		skillratio += 550;
	}
	skillratio += sstatus->dex; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}

int64 SkillSharpenGust::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
