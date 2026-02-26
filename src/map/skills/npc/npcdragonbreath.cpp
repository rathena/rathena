// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcdragonbreath.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillNpcDragonBreath::SkillNpcDragonBreath() : WeaponSkillImpl(NPC_DRAGONBREATH) {
}

void SkillNpcDragonBreath::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	if (skill_lv > 5)
		sc_start4(src,target,SC_FREEZING,50,skill_lv,1000,src->id,0,skill_get_time(getSkillId(),skill_lv));
	else
		sc_start4(src,target,SC_BURNING,50,skill_lv,1000,src->id,0,skill_get_time(getSkillId(),skill_lv));
}

void SkillNpcDragonBreath::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	if (skill_lv > 5)
		base_skillratio += 500 + 500 * (skill_lv - 5);	// Level 6-10 is using water element, like RK_DRAGONBREATH_WATER
	else
		base_skillratio += 500 + 500 * skill_lv;	// Level 1-5 is using fire element, like RK_DRAGONBREATH
}

void SkillNpcDragonBreath::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);

	if( tsc && tsc->getSCE(SC_HIDING) )
		clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
	else {
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	}
}

void SkillNpcDragonBreath::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Cast center might be relevant later (e.g. for knockback direction)
	skill_area_temp[4] = x;
	skill_area_temp[5] = y;
	int32 i = skill_get_splash(getSkillId(),skill_lv);
	map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR|BL_SKILL,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
}
