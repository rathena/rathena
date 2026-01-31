// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "eternalslash.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillEternalSlash::SkillEternalSlash() : WeaponSkillImpl(SHC_ETERNAL_SLASH) {
}

void SkillEternalSlash::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 300 * skill_lv + 2 * sstatus->pow;

	if( sc != nullptr && sc->getSCE( SC_SHADOW_EXCEED ) ){
		skillratio += 120 * skill_lv + sstatus->pow;
	}

	RE_LVL_DMOD(100);
}

void SkillEternalSlash::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);

	if( sc && sc->getSCE(SC_E_SLASH_COUNT) )
		sc_start(src, src, SC_E_SLASH_COUNT, 100, min( 5, 1 + sc->getSCE(SC_E_SLASH_COUNT)->val1 ), skill_get_time(getSkillId(), skill_lv));
	else
		sc_start(src, src, SC_E_SLASH_COUNT, 100, 1, skill_get_time(getSkillId(), skill_lv));
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
