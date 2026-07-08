// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "howlingofchulho.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHowlingofChulho::SkillHowlingofChulho() : SkillImpl(SH_HOWLING_OF_CHUL_HO) {
}

void SkillHowlingofChulho::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillHowlingofChulho::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 600 + 1050 * skill_lv;
	skillratio += 50 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	skillratio += 5 * sstatus->pow;

	if( pc_checkskill( sd, SH_COMMUNE_WITH_CHUL_HO ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) ){
		skillratio += 100 + 100 * skill_lv;
		skillratio += 50 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	}
	RE_LVL_DMOD(100);
}

void SkillHowlingofChulho::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillHowlingofChulho::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST(BL_PC, src);

	int32 range = skill_get_splash(getSkillId(), skill_lv);

	if( pc_checkskill( sd, SH_COMMUNE_WITH_CHUL_HO ) > 0 || ( sc != nullptr && sc->getSCE( SC_TEMPORARY_COMMUNION ) != nullptr ) ){
		range += 1;
	}

	skill_area_temp[0] = 0;
	skill_area_temp[1] = target->id;
	skill_area_temp[2] = 0;
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	map_foreachinrange(skill_area_sub, target, range, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
}
