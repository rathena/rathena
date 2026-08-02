// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "throwhuumashuriken.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"

SkillThrowHuumaShuriken::SkillThrowHuumaShuriken() : SkillImplRecursiveDamageSplash(NJ_HUUMA) {
}

void SkillThrowHuumaShuriken::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += -150 + 250 * skill_lv;
#else
	base_skillratio += 50 + 150 * skill_lv;
#endif
}

void SkillThrowHuumaShuriken::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
#ifdef RENEWAL
	clif_skill_damage( *src, *target,tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
#endif
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
