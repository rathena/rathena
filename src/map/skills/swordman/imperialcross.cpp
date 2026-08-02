// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "imperialcross.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillImperialCross::SkillImperialCross() : WeaponSkillImpl(IG_IMPERIAL_CROSS) {
}

void SkillImperialCross::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillImperialCross::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);

	skillratio += -100 + 1650 + 1350 * skill_lv;
	skillratio += pc_checkskill(sd, IG_SPEAR_SWORD_M) * 25;
	skillratio += 5 * sstatus->pow;	// !TODO: check POW ratio

	if (sc != nullptr && sc->getSCE(SC_SPEAR_SCAR))
		skillratio += 100 + 300 * skill_lv;

	RE_LVL_DMOD(100);
}
