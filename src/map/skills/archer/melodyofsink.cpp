// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "melodyofsink.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMelodyOfSink::SkillMelodyOfSink() : SkillImpl(WM_MELODYOFSINK) {
}

void SkillMelodyOfSink::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( flag&1 ) {
		sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
	} else {	// These affect to all targets around the caster.
		if( rnd()%100 < 5 + 5 * skill_lv + pc_checkskill(sd, WM_LESSON) ) { // !TODO: What's the Lesson bonus?
			map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(),skill_lv),BL_PC, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		}
	}
}
