// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "throwshuriken.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/skill.hpp"
#include "map/unit.hpp"

SkillThrowShuriken::SkillThrowShuriken() : WeaponSkillImpl(NJ_SYURIKEN) {
}

void SkillThrowShuriken::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target,
                                             uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 5 * skill_lv;
#endif
}

void SkillThrowShuriken::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id_, skill_lv, tick, flag);
}
