// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "vituperatum.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillVituperatum::SkillVituperatum() : StatusSkillImpl(AB_VITUPERATUM) {
}

void SkillVituperatum::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag&1)
		StatusSkillImpl::castendNoDamageId(src, target, skill_lv, tick, flag);
	else {
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
