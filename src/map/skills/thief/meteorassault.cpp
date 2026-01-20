// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "meteorassault.hpp"

#include <config/const.hpp>
#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillMeteorAssault::SkillMeteorAssault() : SkillImplRecursiveDamageSplash(ASC_METEORASSAULT) {
}

void SkillMeteorAssault::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
#ifdef RENEWAL
	skillratio += 100 + 120 * skill_lv;
	RE_LVL_DMOD(100);
#else
	skillratio += -60 + 40 * skill_lv;
#endif
}

void SkillMeteorAssault::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Skill give damage around caster
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
	
void SkillMeteorAssault::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	//Any enemies hit by this skill will receive Stun, Darkness, or external bleeding status ailment with a 5%+5*skill_lv% chance.
	switch(rnd()%3) {
		case 0:
			sc_start(src,target,SC_BLIND,(5+skill_lv*5),skill_lv,skill_get_time2(getSkillId(),1));
			break;
		case 1:
			sc_start(src,target,SC_STUN,(5+skill_lv*5),skill_lv,skill_get_time2(getSkillId(),2));
			break;
		default:
			sc_start2(src,target,SC_BLEEDING,(5+skill_lv*5),skill_lv,src->id,skill_get_time2(getSkillId(),3));
	}
}
