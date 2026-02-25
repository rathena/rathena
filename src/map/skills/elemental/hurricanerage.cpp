// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hurricanerage.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

// EL_HURRICANE
SkillHurricaneRage::SkillHurricaneRage() : SkillImpl(EL_HURRICANE) {
}

void SkillHurricaneRage::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 600;
}

void SkillHurricaneRage::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if( flag&1 )
		skill_attack(skill_get_type(EL_HURRICANE_ATK),src,src,target,EL_HURRICANE_ATK,skill_lv,tick,flag);
	else {
		int32 i = skill_get_splash(getSkillId(),skill_lv);
		clif_skill_nodamage(src,*battle_get_master(src),getSkillId(),skill_lv);
		clif_skill_damage( *src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
		if( rnd()%100 < 30 )
			map_foreachinrange(skill_area_sub,target,i,BL_CHAR,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		else
			skill_attack(skill_get_type(getSkillId()),src,src,target,getSkillId(),skill_lv,tick,flag);
	}
}


// EL_HURRICANE_ATK
SkillHurricaneRageAttack::SkillHurricaneRageAttack() : SkillImpl(EL_HURRICANE_ATK) {
}

void SkillHurricaneRageAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 400;
}
