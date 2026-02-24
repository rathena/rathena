// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "roaringpiercer.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

#include "skill_factory_druid.hpp"

// AT_ROARING_PIERCER
SkillRoaringPiercer::SkillRoaringPiercer() : SkillImpl(AT_ROARING_PIERCER) {
}

void SkillRoaringPiercer::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 11250 + 700 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_WIND)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 5 * sstatus->spl;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_WIND
	RE_LVL_DMOD(100);
}

void SkillRoaringPiercer::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_THUNDERING_ROD_MAX)) {
		SkillRoaringPiercerS skill_overcharged;
		skill_overcharged.castendDamageId(src, target, skill_lv, tick, flag);
		return;
	}

	SkillFactoryDruid::addThunderingCharge(src, getSkillId(), skill_lv, 1);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	skill_area_temp[1] = target->id;

	if (battle_config.skill_eightpath_algorithm) {
		// Use official AoE algorithm
		map_foreachindir(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_range(getSkillId(), skill_lv)+2, 0, splash_target(src),
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	} else {
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_range(getSkillId(), skill_lv)+2, splash_target(src),
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	}
}


// AT_ROARING_PIERCER_S
SkillRoaringPiercerS::SkillRoaringPiercerS() : SkillImpl(AT_ROARING_PIERCER_S) {
}

void SkillRoaringPiercerS::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 11250 + 750 * (skill_lv - 1);

	// SPL and BaseLevel ratio do not depend on SC_TRUTH_OF_WIND
	skillratio += 9 * sstatus->spl;

	RE_LVL_DMOD(100);
}

void SkillRoaringPiercerS::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	status_change_end(src, SC_THUNDERING_ROD);
	status_change_end(src, SC_THUNDERING_ROD_MAX);

	skill_area_temp[1] = target->id;

	if (battle_config.skill_eightpath_algorithm) {
		// Use official AoE algorithm
		map_foreachindir(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_range(getSkillId(), skill_lv)+2, 0, splash_target(src),
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	} else {
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, target->x, target->y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_range(getSkillId(), skill_lv)+2, splash_target(src),
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	}
}
