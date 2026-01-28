// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "flickingtornado.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillFlickingTornado::SkillFlickingTornado() : SkillImplRecursiveDamageSplash(DR_FLICKING_TONADO) {
}

void SkillFlickingTornado::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	if (!(flag & 1)) {
		uint8 dir = map_calc_dir(src, target->x, target->y);
		int32 retreat = skill_lv >= 6 ? 5 : 3;
		skill_blown(src, src, retreat, dir, BLOWN_IGNORE_NO_KNOCKBACK);
	}
}

void SkillFlickingTornado::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 100 * skill_lv;
	if (sc != nullptr && sc->hasSCE(SC_ENRAGE_RAPTOR)) {
		skillratio += 50 * skill_lv;
	}
	skillratio += sstatus->dex; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}

int64 SkillFlickingTornado::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag | SD_ANIMATION);
}
