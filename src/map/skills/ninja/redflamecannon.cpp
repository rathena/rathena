// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "redflamecannon.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillRedFlameCannon::SkillRedFlameCannon() : SkillImplRecursiveDamageSplash(SS_SEKIENHOU) {
}

void SkillRedFlameCannon::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 600 + 1100 * skill_lv;
	skillratio += 70 * pc_checkskill( sd, SS_ANTENPOU ) * skill_lv;
	skillratio += 5 * sstatus->spl;

	if( sc != nullptr && sc->hasSCE( SC_FIRE_CHARM_POWER ) ){
		skillratio += 8500;
	}

	RE_LVL_DMOD(100);
}

void SkillRedFlameCannon::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_mirage_cast(*src, nullptr, SS_ANTENPOU, skill_lv, 0, 0, tick, flag | BCT_WOS);
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
