// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hammerofgod.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHammerOfGod::SkillHammerOfGod() : SkillImplRecursiveDamageSplash(RL_HAMMER_OF_GOD) {
}


// TODO: refactor to SkillImplRecursiveDamageSplash
void SkillHammerOfGod::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* tsc = status_get_sc(target);

	if (flag & 1) {
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag | SD_ANIMATION);
		return;
	}

	if (sd && tsc && tsc->getSCE(SC_C_MARKER)) {
		int32 i = 0;

		ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, sd->c_marker[i] == target->id);
		if (i < MAX_SKILL_CRIMSON_MARKER) {
			flag |= 8;
		}
	}

	clif_skill_poseffect(*src, getSkillId(), 1, target->x, target->y, gettick());
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
}

void SkillHammerOfGod::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 100 * skill_lv;
	if (sd) {
		if (wd->miscflag & 8) {
			skillratio += 400 * sd->spiritball_old;
		} else {
			skillratio += 150 * sd->spiritball_old;
		}
	}
	RE_LVL_DMOD(100);
}
