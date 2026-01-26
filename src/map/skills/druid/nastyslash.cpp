// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "nastyslash.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillNastySlash::SkillNastySlash() : SkillImplRecursiveDamageSplash(KR_NASTY_SLASH) {
}

void SkillNastySlash::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
	if (!(flag & 1)) {
		uint8 dir = map_calc_dir(src, target->x, target->y);
		skill_blown(src, src, 5, dir, BLOWN_IGNORE_NO_KNOCKBACK);
	}
}

void SkillNastySlash::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	int32 skillratio = 1150 + 50 * (skill_lv - 1);
	if (sc != nullptr && sc->hasSCE(SC_ENRAGE_WOLF)) {
		skillratio += 300;
	}
	skillratio += sstatus->str; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}

int64 SkillNastySlash::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}
