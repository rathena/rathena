// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hogogongstrike.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHogogongStrike::SkillHogogongStrike() : SkillImpl(SH_HOGOGONG_STRIKE) {
}

void SkillHogogongStrike::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 180 + 200 * skill_lv;
	skillratio += 10 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	skillratio += 5 * sstatus->pow;

	if( pc_checkskill( sd, SH_COMMUNE_WITH_CHUL_HO ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) ){
		skillratio += 70 + 150 * skill_lv;
		skillratio += 10 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	}
	RE_LVL_DMOD(100);
}

void SkillHogogongStrike::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);

	if( flag&1 && tsc != nullptr && tsc->getSCE( SC_HOGOGONG ) != nullptr ){
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	}
}

void SkillHogogongStrike::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( pc_checkskill( sd, SH_COMMUNE_WITH_CHUL_HO ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) )
		status_heal(src, 0, 0, 1, 0);
	skill_area_temp[0] = 0;
	skill_area_temp[1] = target->id;
	skill_area_temp[2] = 0;
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
}
