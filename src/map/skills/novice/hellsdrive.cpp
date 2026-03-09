// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "hellsdrive.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHellsDrive::SkillHellsDrive() : SkillImpl(HN_HELLS_DRIVE) {
}

void SkillHellsDrive::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 1700 + 900 * skill_lv;
	skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 4 * skill_lv;
	skillratio += 3 * sstatus->spl;
	RE_LVL_DMOD(100);
	// After RE_LVL_DMOD calculation, HN_SELFSTUDY_SOCERY amplifies the skill ratio of HN_HELLS_DRIVE by (skill level)%
	skillratio += skillratio * pc_checkskill(sd, HN_SELFSTUDY_SOCERY) / 100;
	// SC_RULEBREAK increases the skill ratio after HN_SELFSTUDY_SOCERY
	if (sc && sc->getSCE(SC_RULEBREAK))
		skillratio += skillratio * 70 / 100;
}

void SkillHellsDrive::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillHellsDrive::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
}
