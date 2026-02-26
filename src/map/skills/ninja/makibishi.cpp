// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "makibishi.hpp"

#include <common/random.hpp>

#include "map/status.hpp"

SkillMakibishi::SkillMakibishi() : SkillImpl(KO_MAKIBISHI) {
}

void SkillMakibishi::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target, SC_STUN, 10 * skill_lv, skill_lv, skill_get_time2(getSkillId(),skill_lv));
}

void SkillMakibishi::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 20 * skill_lv;
}

void SkillMakibishi::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	for( int32 i = 0; i < (skill_lv+2); i++ ) {
		x = src->x - 1 + rnd()%3;
		y = src->y - 1 + rnd()%3;
		skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
	}
}
