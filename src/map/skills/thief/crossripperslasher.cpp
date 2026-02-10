// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crossripperslasher.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCrossRipperSlasher::SkillCrossRipperSlasher() : WeaponSkillImpl(GC_CROSSRIPPERSLASHER) {
}

void SkillCrossRipperSlasher::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 80 * skill_lv + (sstatus->agi * 3);
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_ROLLINGCUTTER))
		skillratio += sc->getSCE(SC_ROLLINGCUTTER)->val1 * 200;
}

void SkillCrossRipperSlasher::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( sd && !(sc && sc->getSCE(SC_ROLLINGCUTTER)) )
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL_CONDITION );
	else
	{
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
	}
}
