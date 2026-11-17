// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "echosong.hpp"

#include "map/clif.hpp"
#include "map/party.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEchoSong::SkillEchoSong() : SkillImpl(MI_ECHOSONG) {
}

void SkillEchoSong::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* sd = BL_CAST(BL_PC, src);
	uint16 lesson_lv = (sd != nullptr) ? pc_checkskill(sd, WM_LESSON) : skill_get_max(WM_LESSON);

	if( sd == nullptr || sd->status.party_id == 0 || (flag & 1) ) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		sc_start2(src, target, type, 100, skill_lv, lesson_lv, skill_get_time(getSkillId(), skill_lv));
	} else {
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(getSkillId(), skill_lv), src, getSkillId(), skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		sc_start2(src, target, type, 100, skill_lv, lesson_lv, skill_get_time(getSkillId(), skill_lv));
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
