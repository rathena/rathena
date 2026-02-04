// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "servantweaponphantom.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillServantWeaponPhantom::SkillServantWeaponPhantom() : SkillImplRecursiveDamageSplash(DK_SERVANT_W_PHANTOM) {
}

void SkillServantWeaponPhantom::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 200 + 300 * skill_lv + 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillServantWeaponPhantom::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_HANDICAPSTATE_DEEPBLIND, 30 + 10 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillServantWeaponPhantom::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// Jump to the target before attacking.
	if (skill_check_unit_movepos(5, src, target->x, target->y, 0, 1))
		skill_blown(src, src, 1, (map_calc_dir(target, src->x, src->y) + 4) % 8, BLOWN_NONE);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);// Trigger animation on servants.

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
