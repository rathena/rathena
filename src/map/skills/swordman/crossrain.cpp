// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crossrain.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillCrossRain::SkillCrossRain() : SkillImpl(IG_CROSS_RAIN) {
}

void SkillCrossRain::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).

	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillCrossRain::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);

	if( sc && sc->getSCE( SC_HOLY_S ) ){
		skillratio += -100 + ( 650 + 15 * pc_checkskill( sd, IG_SPEAR_SWORD_M ) ) * skill_lv;
	}else{
		skillratio += -100 + ( 450 + 10 * pc_checkskill( sd, IG_SPEAR_SWORD_M ) ) * skill_lv;
	}
	skillratio += 7 * sstatus->spl;
	RE_LVL_DMOD(100);
}
