// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "unluckyrush.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillUnluckyRush::SkillUnluckyRush() : WeaponSkillImpl(ABC_UNLUCKY_RUSH) {
}

void SkillUnluckyRush::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Jump to the target before attacking.
	if (skill_check_unit_movepos(5, src, target->x, target->y, 0, 1))
		skill_blown(src, src, 1, (map_calc_dir(target, src->x, src->y) + 4) % 8, BLOWN_NONE);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillUnluckyRush::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);

	skillratio += -100 + 100 + 300 * skill_lv + 5 * sstatus->pow;
	if (sc != nullptr && sc->hasSCE(SC_CHASING))
		skillratio += 2500 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillUnluckyRush::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_HANDICAPSTATE_MISFORTUNE, 30 + 10 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
