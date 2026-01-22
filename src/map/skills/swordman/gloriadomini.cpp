// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gloriadomini.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillGloriaDomini::SkillGloriaDomini() : SkillImpl(PA_PRESSURE) {
}

void SkillGloriaDomini::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
#ifdef RENEWAL
	skillratio += -100 + 500 + 150 * skill_lv;
	RE_LVL_DMOD(100);
#endif
}

void SkillGloriaDomini::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
#ifdef RENEWAL
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
#else
	skill_attack(skill_get_type(getSkillId()),src,src,target,getSkillId(),skill_lv,tick,flag);
#endif
}

void SkillGloriaDomini::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
#ifndef RENEWAL
	status_percent_damage(src, target, 0, 15+5*skill_lv, false);
	//Pressure can trigger physical autospells
	attack_type |= BF_NORMAL;
	attack_type |= BF_WEAPON;
#endif
}
