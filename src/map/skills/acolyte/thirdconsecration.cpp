// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thirdconsecration.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillThirdConsecration::SkillThirdConsecration() : SkillImplRecursiveDamageSplash(IQ_THIRD_CONSECRATION) {
}

void SkillThirdConsecration::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 700 * skill_lv + 10 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillThirdConsecration::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change_end(target, SC_SECOND_BRAND);
}

void SkillThirdConsecration::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	status_heal(src, status_get_max_hp(src) * skill_lv / 100, status_get_max_sp(src) * skill_lv / 100, 0);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
