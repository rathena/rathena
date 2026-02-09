// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thirdflamebomb.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillThirdFlameBomb::SkillThirdFlameBomb() : SkillImplRecursiveDamageSplash(IQ_THIRD_FLAME_BOMB) {
}

void SkillThirdFlameBomb::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 650 * skill_lv + 10 * sstatus->pow;
	skillratio += sstatus->max_hp * 20 / 100;
	RE_LVL_DMOD(100);
}

void SkillThirdFlameBomb::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change_end(target, SC_SECOND_BRAND);
}

void SkillThirdFlameBomb::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	if (sd && sd->spiritball / 5 > 1)
		skill_area_temp[0] = sd->spiritball / 5 - 1;

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
