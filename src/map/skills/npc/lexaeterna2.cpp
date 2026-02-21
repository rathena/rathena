// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lexaeterna2.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"

SkillLexAeterna2::SkillLexAeterna2() : SkillImpl(NPC_LEX_AETERNA) {
}

void SkillLexAeterna2::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea(skill_area_sub, src->m, x-i, y-i, x+i, y+i, BL_CHAR, src, PR_LEXAETERNA, 1, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
}
