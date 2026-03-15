// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "silentium.hpp"

#include "map/clif.hpp"

SkillSilentium::SkillSilentium() : SkillImpl(AB_SILENTIUM) {
}

void SkillSilentium::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Should the level of Lex Divina be equivalent to the level of Silentium or should the highest level learned be used? [LimitLine]
	map_foreachinallrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_CHAR,
		src, PR_LEXDIVINA, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
}
