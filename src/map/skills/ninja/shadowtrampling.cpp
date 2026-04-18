// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shadowtrampling.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillShadowTrampling::SkillShadowTrampling() : SkillImpl(KG_KAGEHUMI) {
}

void SkillShadowTrampling::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());

	if( flag&1 ){
		if (target->type != BL_PC)
			return;
		if (tsc && (tsc->option & (OPTION_CLOAK | OPTION_HIDE) || tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC__SHADOWFORM) || tsc->getSCE(SC_MARIONETTE) || tsc->getSCE(SC_HARMONIZE))) {
				status_change_end(target, SC_HIDING);
				status_change_end(target, SC_CLOAKING);
				status_change_end(target, SC_CLOAKINGEXCEED);
				status_change_end(target, SC_CAMOUFLAGE);
				status_change_end(target, SC_NEWMOON);
				if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
					status_change_end(target, SC__SHADOWFORM);
				status_change_end(target, SC_MARIONETTE);
				status_change_end(target, SC_HARMONIZE);
				sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
		}
	}else{
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR|BL_SKILL, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
