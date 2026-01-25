// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "roaringpiercer.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

SkillRoaringPiercer::SkillRoaringPiercer() : SkillImplRecursiveDamageSplash(AT_ROARING_PIERCER) {
}

void SkillRoaringPiercer::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (flag & 1) {
		SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	} else {
		const int16 half_width = 2;
		const int16 half_length = 4;
		const int32 length = half_length * 2 + 1;
		int32 offset = distance(target->x - src->x, target->y - src->y) - half_length;

		if (offset < 0) {
			offset = 0;
		}

		skill_area_temp[0] = 0;
		skill_area_temp[1] = target->id;
		skill_area_temp[2] = 0;
		map_foreachindir(skill_area_sub, src->m, src->x, src->y, target->x, target->y, half_width, length, offset, splash_target(src),
					src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | SD_SPLASH | 1, skill_castend_damage_id);
	}
	if (!(flag & 1)) {
		try_gain_thundering_charge(src, sc, getSkillId(), 1);
	}
}

void SkillRoaringPiercer::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 11250 + 700 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillRoaringPiercer::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	e_skill actual_skill = getSkillId();
	const status_change* sc = status_get_sc(src);
	actual_skill = resolve_thundering_charge_skill(sc, actual_skill);

	return skill_attack(skill_get_type(actual_skill), src, src, target, actual_skill, skill_lv, tick, flag);
}

SkillRoaringPiercerS::SkillRoaringPiercerS() : SkillImplRecursiveDamageSplash(AT_ROARING_PIERCER_S) {
}

void SkillRoaringPiercerS::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (flag & 1) {
		SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	} else {
		const int16 half_width = 2;
		const int16 half_length = 4;
		const int32 length = half_length * 2 + 1;
		int32 offset = distance(target->x - src->x, target->y - src->y) - half_length;

		if (offset < 0) {
			offset = 0;
		}

		skill_area_temp[0] = 0;
		skill_area_temp[1] = target->id;
		skill_area_temp[2] = 0;
		map_foreachindir(skill_area_sub, src->m, src->x, src->y, target->x, target->y, half_width, length, offset, splash_target(src),
					src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | SD_SPLASH | 1, skill_castend_damage_id);
	}
}

void SkillRoaringPiercerS::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 11250 + 750 * (skill_lv - 1);
	if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
		RE_LVL_DMOD(100);
	}
	base_skillratio += -100 + skillratio;
}

int64 SkillRoaringPiercerS::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
