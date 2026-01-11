// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "landmine.hpp"

#include "map/status.hpp"

SkillLandMine::SkillLandMine() : SkillImpl(HT_LANDMINE) {
}

void SkillLandMine::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_STUN, 10, skill_lv, skill_get_time2(getSkillId(), skill_lv), 1000);
}
