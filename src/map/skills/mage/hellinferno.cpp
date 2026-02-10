// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hellinferno.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillHellInferno::SkillHellInferno() : SkillImpl(WL_HELLINFERNO) {
}

void SkillHellInferno::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 400 * skill_lv;
	if (mflag & 2) // ELE_DARK
		skillratio += 200 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillHellInferno::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1) {
		skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
		skill_addtimerskill(src, tick + 300, target->id, 0, 0, getSkillId(), skill_lv, BF_MAGIC, flag | 2);
	} else {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
	}
}
