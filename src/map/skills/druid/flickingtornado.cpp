// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "flickingtornado.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillFlickingTornado::SkillFlickingTornado() : SkillImplRecursiveDamageSplash(DR_FLICKING_TONADO) {
}

void SkillFlickingTornado::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 100 * skill_lv;

	if (sc != nullptr && sc->hasSCE(SC_ENRAGE_RAPTOR)) {
		skillratio += 50 * skill_lv;
	}

	skillratio += 5 * sstatus->dex;

	RE_LVL_DMOD(100);
}

void SkillFlickingTornado::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);

	// TODO : the direction should be opposite to the direction the player is facing (before casting the skill)

	uint8 dir = map_calc_dir(src, target->x, target->y);
	int32 retreat = skill_lv >= 6 ? 5 : 3;
	skill_blown(src, src, retreat, dir, BLOWN_IGNORE_NO_KNOCKBACK);
}