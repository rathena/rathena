// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "magmaeruption.hpp"

#include "map/battle.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

// NC_MAGMA_ERUPTION
SkillMagmaEruption::SkillMagmaEruption() : WeaponSkillImpl(NC_MAGMA_ERUPTION) {
}

void SkillMagmaEruption::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Stun effect from 'slam'
	sc_start(src, target, SC_STUN, 90, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillMagmaEruption::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	// 'Slam' damage
	base_skillratio += 350 + 50 * skill_lv;
}

void SkillMagmaEruption::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// 1st, AoE 'slam' damage
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinarea(skill_area_sub, src->m, x-i, y-i, x+i, y+i, BL_CHAR,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_ANIMATION|1, skill_castend_damage_id);
	// 2nd, AoE 'eruption' unit
	skill_addtimerskill(src,tick + status_get_amotion(src) * 2,0,x,y,getSkillId(),skill_lv,0,flag);
}


// NC_MAGMA_ERUPTION_DOTDAMAGE
SkillMagmaEruptionDotDamage::SkillMagmaEruptionDotDamage() : SkillImpl(NC_MAGMA_ERUPTION_DOTDAMAGE) {
}

void SkillMagmaEruptionDotDamage::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Burning effect from 'eruption'
	sc_start4(src, target, SC_BURNING, 10 * skill_lv, skill_lv, 1000, src->id, 0, skill_get_time2(getSkillId(), skill_lv));
}
