// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "dragonicpierce.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillDragonicPierce::SkillDragonicPierce() : WeaponSkillImpl(DK_DRAGONIC_PIERCE) {
}

void SkillDragonicPierce::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillDragonicPierce::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 850 + 600 * skill_lv;
	skillratio += 7 * sstatus->pow;	// !TODO: unknown ratio

	if (sc != nullptr && sc->hasSCE(SC_DRAGONIC_AURA))
		skillratio += 100 + 50 * skill_lv;

	RE_LVL_DMOD(100);
}
