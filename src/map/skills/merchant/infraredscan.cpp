// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "infraredscan.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillInfraredScan::SkillInfraredScan() : SkillImpl(NC_INFRAREDSCAN) {
}

void SkillInfraredScan::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* tsc = status_get_sc(target);

	if (flag & 1) {
		status_change_end(target, SC_HIDING);
		status_change_end(target, SC_CLOAKING);
		status_change_end(target, SC_CLOAKINGEXCEED);
		status_change_end(target, SC_CAMOUFLAGE);
		status_change_end(target, SC_NEWMOON);
		if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) {// [100 - (Skill Level x 10)] %
			status_change_end(target, SC__SHADOWFORM);
		}
		sc_start(src, target, SC_INFRAREDSCAN, 10000, skill_lv, skill_get_time(getSkillId(), skill_lv));
	} else {
		clif_skill_damage(*src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE);
		map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), splash_target(src), src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
	}
}

void SkillInfraredScan::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_castend_damage_id(src, src, getSkillId(), skill_lv, tick, flag);
}
