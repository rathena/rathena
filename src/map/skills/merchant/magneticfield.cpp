// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "magneticfield.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillMagneticField::SkillMagneticField() : SkillImpl(NC_MAGNETICFIELD) {
}

void SkillMagneticField::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1) {
		sc_start2(src, target, SC_MAGNETICFIELD, 100, skill_lv, src->id, skill_get_time(getSkillId(), skill_lv));
	} else {
		if (map_flag_vs(src->m)) // Doesn't affect the caster in non-PVP maps [exneval]
			sc_start2(src, target, skill_get_sc(getSkillId()), 100, skill_lv, src->id, skill_get_time(getSkillId(), skill_lv));
		map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), splash_target(src), src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_nodamage_id);
		clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	}
}
