// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hyunrokspiritpower.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHyunrokSpiritPower::SkillHyunrokSpiritPower() : SkillImplRecursiveDamageSplash(SH_HYUN_ROK_SPIRIT_POWER) {
}

void SkillHyunrokSpiritPower::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 350 + 200 * skill_lv;
	skillratio += 30 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillHyunrokSpiritPower::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

void SkillHyunrokSpiritPower::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
	const status_change* sc = status_get_sc(&src);

	if( sc != nullptr && !sc->empty() ){
		if( sc->hasSCE( SC_COLORS_OF_HYUN_ROK_1 ) ){
			element = ELE_WATER;
		}else if( sc->hasSCE( SC_COLORS_OF_HYUN_ROK_2 ) ){
			element = ELE_WIND;
		}else if( sc->hasSCE( SC_COLORS_OF_HYUN_ROK_3 ) ){
			element = ELE_EARTH;
		}else if( sc->hasSCE( SC_COLORS_OF_HYUN_ROK_4 ) ){
			element = ELE_FIRE;
		}else if( sc->hasSCE( SC_COLORS_OF_HYUN_ROK_5 ) ){
			element = ELE_DARK;
		}else if( sc->hasSCE( SC_COLORS_OF_HYUN_ROK_6 ) ){
			element = ELE_HOLY;
		}
	}
}
