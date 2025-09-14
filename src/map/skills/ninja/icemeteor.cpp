// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "icemeteor.hpp"

#include "map/status.hpp"

SkillIceMeteor::SkillIceMeteor() : SkillImpl(NJ_HYOUSYOURAKU) {
}

void SkillIceMeteor::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_FREEZE, (10 + 10 * skill_lv), skill_lv, skill_get_time2(this->skill_id_, skill_lv));
}
