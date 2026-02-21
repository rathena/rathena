// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shootingfeather.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillShootingFeather::SkillShootingFeather() : WeaponSkillImpl(DR_SHOOTING_FEATHER) {
}

void SkillShootingFeather::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 20 * skill_lv;

	if (sc != nullptr && sc->hasSCE(SC_ENRAGE_RAPTOR)) {
		skillratio += 20 * skill_lv;
	}

	skillratio += 3 * sstatus->dex;

	RE_LVL_DMOD(100);
}

void SkillShootingFeather::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
