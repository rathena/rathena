// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "bodypainting.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillBodyPainting::SkillBodyPainting() : SkillImpl(SC_BODYPAINT) {
}

void SkillBodyPainting::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());

	if( flag&1 ) {
		if (tsc && ((tsc->option&(OPTION_HIDE|OPTION_CLOAK)) || tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC_STEALTHFIELD))) {
			status_change_end(target,SC_HIDING);
			status_change_end(target,SC_CLOAKING);
			status_change_end(target,SC_CLOAKINGEXCEED);
			status_change_end(target,SC_CAMOUFLAGE);
			status_change_end(target,SC_NEWMOON);
			if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
				status_change_end(target, SC__SHADOWFORM);
		}
		// Attack Speed decrease and Blind happen to everyone around caster, not just hidden targets.
		sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		sc_start(src, target, SC_BLIND, 53 + 2 * skill_lv, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	} else {
		clif_skill_nodamage(src, *target, getSkillId(), 0);
		map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
	}
}
