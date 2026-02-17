// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "blazingflameblast.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillBlazingFlameBlast::SkillBlazingFlameBlast() : WeaponSkillImpl(IQ_BLAZING_FLAME_BLAST) {
}

void SkillBlazingFlameBlast::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillBlazingFlameBlast::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);

	skillratio += -100 + 2000 + 3800 * skill_lv;
	skillratio += 10 * sstatus->pow;	// !TODO: unknown ratio
	if (sc != nullptr && sc->hasSCE(SC_MASSIVE_F_BLASTER))
		skillratio += 1500 + 400 * skill_lv;
	RE_LVL_DMOD(100);
}
