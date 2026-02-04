// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "astralstrike.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

// AG_ASTRAL_STRIKE
SkillAstralStrike::SkillAstralStrike() : SkillImpl(AG_ASTRAL_STRIKE) {
}

void SkillAstralStrike::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 300 + 1800 * skill_lv + 10 * sstatus->spl;
	if (tstatus->race == RC_UNDEAD || tstatus->race == RC_DRAGON)
		skillratio += 100 + 300 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillAstralStrike::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillAstralStrike::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 i = skill_get_splash(getSkillId(), skill_lv);
	map_foreachinallarea(skill_area_sub, src->m, x-i, y-i, x+i, y+i, BL_CHAR, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
	flag |= 1;
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
}


// AG_ASTRAL_STRIKE_ATK
SkillAstralStrikeAttack::SkillAstralStrikeAttack() : SkillImpl(AG_ASTRAL_STRIKE_ATK) {
}

void SkillAstralStrikeAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 650 * skill_lv + 10 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillAstralStrikeAttack::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}
