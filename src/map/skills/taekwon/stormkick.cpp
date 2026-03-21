// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stormkick.hpp"

#include "map/clif.hpp"

SkillStormKick::SkillStormKick() : SkillImpl(TK_STORMKICK) {
}

void SkillStormKick::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 60 + 20 * skill_lv;
}

void SkillStormKick::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	skill_area_temp[1] = 0;
	map_foreachinshootrange(skill_attack_area, src,
	                        skill_get_splash(getSkillId(), skill_lv), BL_CHAR | BL_SKILL,
	                        BF_WEAPON, src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
}
