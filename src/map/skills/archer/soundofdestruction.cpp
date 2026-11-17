// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "soundofdestruction.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSoundOfDestruction::SkillSoundOfDestruction() : SkillImpl(WM_SOUND_OF_DESTRUCTION) {
}

void SkillSoundOfDestruction::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag&1) {
		sc_start(src, target, type, 100, skill_lv, (sd ? pc_checkskill(sd, WM_LESSON) * 500 : 0) + skill_get_time(getSkillId(), skill_lv)); // !TODO: Confirm Lesson increase
	} else {
		map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv),BL_PC, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
