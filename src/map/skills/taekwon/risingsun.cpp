// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "risingsun.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillRisingSun::SkillRisingSun() : SkillImpl(SKE_RISING_SUN) {
}

void SkillRisingSun::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);

	if ( sc == nullptr || ( sc->getSCE( SC_RISING_SUN ) == nullptr && sc->getSCE( SC_NOON_SUN ) == nullptr && sc->getSCE( SC_SUNSET_SUN ) == nullptr ) ){
		sc_start(src, src, SC_RISING_SUN, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}else if( sc->getSCE( SC_NOON_SUN ) == nullptr && sc->getSCE( SC_SUNSET_SUN ) == nullptr ){
		sc_start(src, src, SC_NOON_SUN, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}else if( sc->getSCE( SC_SUNSET_SUN ) == nullptr ){
		sc_start(src, src, SC_SUNSET_SUN, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}
}

void SkillRisingSun::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 500 + 600 * skill_lv;
	skillratio += pc_checkskill(sd, SKE_SKY_MASTERY) * 5 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
