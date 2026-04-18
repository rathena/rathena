// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "monsterchant.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"

SkillMonsterChant::SkillMonsterChant() : SkillImpl(SA_SUMMONMONSTER) {
}

void SkillMonsterChant::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if (sd)
		mob_once_spawn(sd, src->m, src->x, src->y,"--ja--", -1, 1, "", SZ_SMALL, AI_NONE);
}
