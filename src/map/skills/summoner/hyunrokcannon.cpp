// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hyunrokcannon.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHyunrokCannon::SkillHyunrokCannon() : SkillImpl(SH_HYUN_ROK_CANNON) {
}

void SkillHyunrokCannon::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 1100 + 2050 * skill_lv;
	skillratio += 50 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	skillratio += 5 * sstatus->spl;

	if( pc_checkskill( sd, SH_COMMUNE_WITH_HYUN_ROK ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) ){
		skillratio += 400 * skill_lv;
		skillratio += 25 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	}
	RE_LVL_DMOD(100);
}

void SkillHyunrokCannon::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillHyunrokCannon::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
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
