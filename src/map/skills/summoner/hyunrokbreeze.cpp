// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hyunrokbreeze.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillHyunrokBreeze::SkillHyunrokBreeze() : SkillImpl(SH_HYUN_ROKS_BREEZE) {
}

void SkillHyunrokBreeze::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 650 + 750 * skill_lv;
	skillratio += 20 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	skillratio += 5 * sstatus->spl;

	if( pc_checkskill( sd, SH_COMMUNE_WITH_HYUN_ROK ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) ){
		skillratio += 100 + 200 * skill_lv;
		skillratio += 20 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	}
	RE_LVL_DMOD(100);
}

void SkillHyunrokBreeze::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
