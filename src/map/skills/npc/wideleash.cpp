// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wideleash.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/unit.hpp"

SkillWideLeash::SkillWideLeash() : SkillImpl(NPC_WIDELEASH) {
}

void SkillWideLeash::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if( flag & 1 ){
		if( !skill_check_unit_movepos( 0, target, src->x, src->y, 1, 1 ) ){
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}

		clif_blown( target );
	}else{
		skill_area_temp[2] = 0; // For SD_PREAMBLE
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinallrange( skill_area_sub, target, skill_get_splash( getSkillId(), skill_lv ), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | 1, skill_castend_nodamage_id );
	}
}
