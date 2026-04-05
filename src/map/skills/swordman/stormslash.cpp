// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stormslash.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillStormSlash::SkillStormSlash() : WeaponSkillImpl(DK_STORMSLASH) {
}

void SkillStormSlash::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillStormSlash::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 300 + 750 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_GIANTGROWTH) && rnd_chance(60, 100))
		skillratio *= 2;
}
