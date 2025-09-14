// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "throwhuumashuriken.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/skill.hpp"
#include "map/unit.hpp"

SkillThrowHuumaShuriken::SkillThrowHuumaShuriken() : WeaponSkillImpl(NJ_HUUMA) {
}

void SkillThrowHuumaShuriken::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio) const {
#ifdef RENEWAL
	base_skillratio += -150 + 250 * skill_lv;
#else
	base_skillratio += 50 + 150 * skill_lv;
#endif
}

void SkillThrowHuumaShuriken::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const {
#ifndef RENEWAL
	if (skill_get_inf2(this->skill_id_, INF2_ISNPC)) {
#endif
		clif_skill_damage(*src, *target, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, this->skill_id_, skill_lv, DMG_SINGLE);
#ifndef RENEWAL
	}
#endif
}
