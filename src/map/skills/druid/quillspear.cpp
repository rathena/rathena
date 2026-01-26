// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "quillspear.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

#include "skill_factory_druid.hpp"

SkillQuillSpear::SkillQuillSpear() : SkillImplRecursiveDamageSplash(AT_QUILL_SPEAR) {
}

void SkillQuillSpear::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (flag & 1) {
		SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	} else {
		const bool apex = sc && sc->hasSCE(SC_APEX_PHASE);
		const int16 half_width = (apex) ? 2 : 1;
		const int16 half_length = 3;
		const int32 length = half_length * 2 + 1;
		const uint8 dir = map_calc_dir(src, target->x, target->y);
		const int16 dir_x = dirx[dir];
		const int16 dir_y = diry[dir];
		const int16 start_x = target->x - dir_x * half_length;
		const int16 start_y = target->y - dir_y * half_length;
		const int16 end_x = target->x;
		const int16 end_y = target->y;

		skill_area_temp[0] = 0;
		skill_area_temp[1] = target->id;
		skill_area_temp[2] = 0;
		map_foreachindir(skill_area_sub, src->m, start_x, start_y, end_x, end_y, half_width, length, 0, splash_target(src),
					src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | SD_SPLASH | 1, skill_castend_damage_id);
	}
}

void SkillQuillSpear::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 2050 * skill_lv;
	skillratio += sstatus->con * 5; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}

int64 SkillQuillSpear::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	const status_change* sc = status_get_sc(src);
	e_skill actual_skill = SkillFactoryDruid::resolve_quill_spear_skill(sc, getSkillId());
	return skill_attack(skill_get_type(actual_skill), src, src, target, actual_skill, skill_lv, tick, flag | SD_ANIMATION);
}

SkillQuillSpearS::SkillQuillSpearS() : SkillImplRecursiveDamageSplash(AT_QUILL_SPEAR_S) {
}

void SkillQuillSpearS::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (flag & 1) {
		SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	} else {
		const int16 half_width = 2;
		const int16 half_length = 3;
		const int32 length = half_length * 2 + 1;
		const uint8 dir = map_calc_dir(src, target->x, target->y);
		const int16 dir_x = dirx[dir];
		const int16 dir_y = diry[dir];
		const int16 start_x = target->x - dir_x * half_length;
		const int16 start_y = target->y - dir_y * half_length;
		const int16 end_x = target->x;
		const int16 end_y = target->y;

		skill_area_temp[0] = 0;
		skill_area_temp[1] = target->id;
		skill_area_temp[2] = 0;
		map_foreachindir(skill_area_sub, src->m, start_x, start_y, end_x, end_y, half_width, length, 0, splash_target(src),
					src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | SD_SPLASH | 1, skill_castend_damage_id);
	}
}

void SkillQuillSpearS::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 2050 * skill_lv;
	skillratio += sstatus->con * 5; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}

int64 SkillQuillSpearS::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag | SD_ANIMATION);
}
