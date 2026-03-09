// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "arrullo.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillArrullo::SkillArrullo() : SkillImpl(SO_ARRULLO) {
}

void SkillArrullo::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	int32 rate = (15 + 5 * skill_lv) + status_get_int(src) / 5 + (sd ? sd->status.job_level / 5 : 0) - status_get_int(target) / 6 - status_get_luk(target) / 10;

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	sc_start(src, target, skill_get_sc(getSkillId()), rate, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillArrullo::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(),skill_lv);
	map_foreachinallarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
}
