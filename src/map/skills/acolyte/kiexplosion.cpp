// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kiexplosion.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillKiExplosion::SkillKiExplosion() : SkillImpl(MO_BALKYOUNG) {
}

void SkillKiExplosion::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Passive part of the attack. Splash knock-back+stun. [Skotlex]
	if (skill_area_temp[1] != target->id) {
		skill_blown(src,target,skill_get_blewcount(getSkillId(),skill_lv),-1,BLOWN_NONE);
		skill_additional_effect(src,target,getSkillId(),skill_lv,BF_MISC,ATK_DEF,tick); //Use Misc rather than weapon to signal passive pushback
	}
}

void SkillKiExplosion::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Active part of the attack. Skill-attack [Skotlex]
	skill_area_temp[1] = target->id; //NOTE: This is used in skill_castend_nodamage_id to avoid affecting the target.
	if (skill_attack(BF_WEAPON,src,src,target,getSkillId(),skill_lv,tick,flag))
		map_foreachinallrange(skill_area_sub,target,
			skill_get_splash(getSkillId(), skill_lv),BL_CHAR,
			src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,
			skill_castend_nodamage_id);
}

void SkillKiExplosion::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 700;
#else
	base_skillratio += 200;
#endif
}

void SkillKiExplosion::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	//Note: attack_type is passed as BF_WEAPON for the actual target, BF_MISC for the splash-affected mobs.
	if(attack_type&BF_MISC) //70% base stun chance...
		sc_start(src,target,SC_STUN,70,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}

