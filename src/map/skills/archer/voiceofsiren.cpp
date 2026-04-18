// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "voiceofsiren.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillVoiceOfSiren::SkillVoiceOfSiren() : SkillImpl(WM_VOICEOFSIREN) {
}

void SkillVoiceOfSiren::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag&1)
		sc_start2(src,target,type,skill_area_temp[5],skill_lv,src->id,skill_area_temp[6]);
	else {
		// Success chance: (Skill Level x 6) + (Voice Lesson Skill Level x 2) + (Caster's Job Level / 2) %
		skill_area_temp[5] = skill_lv * 6 + ((sd) ? pc_checkskill(sd, WM_LESSON) : 1) * 2 + (sd ? sd->status.job_level : 50) / 2;
		skill_area_temp[6] = skill_get_time(getSkillId(),skill_lv);
		map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(),skill_lv), BL_CHAR|BL_SKILL, src, getSkillId(), skill_lv, tick, flag|BCT_ALL|BCT_WOS|1, skill_castend_nodamage_id);
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	}
}
