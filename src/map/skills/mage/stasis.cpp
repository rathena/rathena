// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stasis.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillStasis::SkillStasis() : SkillImpl(WL_STASIS) {
}

void SkillStasis::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	if (flag&1)
		sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
	else {
		struct map_data *mapdata = map_getmapdata(src->m);

		map_foreachinallrange(skill_area_sub,src,skill_get_splash(getSkillId(), skill_lv),BL_CHAR,src,getSkillId(),skill_lv,tick,(mapdata_flag_vs(mapdata)?BCT_ALL:BCT_ENEMY|BCT_SELF)|flag|1,skill_castend_nodamage_id);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
