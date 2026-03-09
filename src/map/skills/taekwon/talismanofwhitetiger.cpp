// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "talismanofwhitetiger.hpp"

#include <config/const.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillTalismanOfWhiteTiger::SkillTalismanOfWhiteTiger() : SkillImplRecursiveDamageSplash(SOA_TALISMAN_OF_WHITE_TIGER) {
}

void SkillTalismanOfWhiteTiger::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 400 + 1000 * skill_lv;
	skillratio += pc_checkskill(sd, SOA_TALISMAN_MASTERY) * 15 * skill_lv;
	skillratio += 5 * sstatus->spl;
	if (sc != nullptr && sc->getSCE(SC_T_FIFTH_GOD) != nullptr)
		skillratio += 400 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillTalismanOfWhiteTiger::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);

	int32 starget = BL_CHAR|BL_SKILL;

	if (sc != nullptr && sc->getSCE(SC_T_FIRST_GOD) != nullptr) {
		sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	}

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
