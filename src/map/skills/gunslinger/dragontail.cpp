// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dragontail.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillDragonTail::SkillDragonTail() : SkillImplRecursiveDamageSplash(RL_D_TAIL) {
}

int32 SkillDragonTail::getSplashTarget(block_list* src) const {
	return BL_CHAR;
}

int64 SkillDragonTail::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag | SD_ANIMATION);
}

void SkillDragonTail::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* tsc = status_get_sc(target);

	if (sd != nullptr && tsc != nullptr && tsc->hasSCE(SC_C_MARKER)) {
		int32 i = 0;

		ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, sd->c_marker[i] == target->id);
		if (i < MAX_SKILL_CRIMSON_MARKER) {
			flag |= 8;
		}
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

void SkillDragonTail::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 500 + 200 * skill_lv;

	if (const map_session_data* sd = BL_CAST(BL_PC, src); sd != nullptr && (wd->miscflag & 8)) {
		skillratio *= 2;
	}

	RE_LVL_DMOD(100);
}
