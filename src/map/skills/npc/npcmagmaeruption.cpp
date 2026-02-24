// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcmagmaeruption.hpp"

#include "map/map.hpp"
#include "map/status.hpp"

SkillNpcMagmaEruption::SkillNpcMagmaEruption() : WeaponSkillImpl(NPC_MAGMA_ERUPTION) {
}

void SkillNpcMagmaEruption::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Stun effect from 'slam'
	sc_start(src, target, SC_STUN, 90, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillNpcMagmaEruption::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// 1st, AoE 'slam' damage
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinarea(skill_area_sub, src->m, x-i, y-i, x+i, y+i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_ANIMATION|1, skill_castend_damage_id);
	// 2nd, AoE 'eruption' unit
	skill_addtimerskill(src,tick + status_get_amotion(src) * 2,0,x,y,getSkillId(),skill_lv,0,flag);
}
