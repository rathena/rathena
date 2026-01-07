// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "throwkunai.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/skill.hpp"
#include "map/unit.hpp"
#include "../weapon_skill_impl.hpp"

SkillThrowKunai::SkillThrowKunai() : WeaponSkillImpl(NJ_KUNAI) {
}

void SkillThrowKunai::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 100 * skill_lv;
}

void SkillThrowKunai::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_WEAPON, src, src, target, this->skill_id_, skill_lv, tick, flag);
}
