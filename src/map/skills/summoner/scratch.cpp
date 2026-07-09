// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "scratch.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillScratch::SkillScratch() : SkillImplRecursiveDamageSplash(SU_SCRATCH) {
}

void SkillScratch::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src, target, SC_BLEEDING, skill_lv * 10 + 70, skill_lv, src->id, skill_get_time(getSkillId(), skill_lv));
}

void SkillScratch::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -50 + 50 * skill_lv;
}

void SkillScratch::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
