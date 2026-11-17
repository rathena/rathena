// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "throwvenomknife.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillThrowVenomKnife::SkillThrowVenomKnife() : WeaponSkillImpl(AS_VENOMKNIFE) {
}

void SkillThrowVenomKnife::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 400;
#endif
}

void SkillThrowVenomKnife::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src, target, SC_POISON, 100, skill_lv, src->id, skill_get_time2(getSkillId(), skill_lv));
}
