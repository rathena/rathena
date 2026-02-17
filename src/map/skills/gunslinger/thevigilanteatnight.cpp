// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thevigilanteatnight.hpp"

#include <config/core.hpp>

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTheVigilanteAtNight::SkillTheVigilanteAtNight() : SkillImpl(NW_THE_VIGILANTE_AT_NIGHT) {
}

void SkillTheVigilanteAtNight::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_change* sc = status_get_sc(src);

	int32 i = skill_get_splash(getSkillId(), skill_lv);
	skill_area_temp[0] = 0;
	skill_area_temp[1] = target->id;
	skill_area_temp[2] = 0;

	if (sd && sd->weapontype1 == W_GATLING) {
		i = 5; // 11x11
		clif_skill_nodamage(src, *target, NW_THE_VIGILANTE_AT_NIGHT_GUN_GATLING, skill_lv);
	} else
		clif_skill_nodamage(src, *target, NW_THE_VIGILANTE_AT_NIGHT_GUN_SHOTGUN, skill_lv);
	map_foreachinrange(skill_area_sub, target, i, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
	if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
		status_change_end(src, SC_INTENSIVE_AIM_COUNT);
}

void SkillTheVigilanteAtNight::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillTheVigilanteAtNight::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	if (sd && sd->weapontype1 == W_GATLING) {
		skillratio += -100 + 300 * skill_lv;
		if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
			skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 100 * skill_lv;
	} else {
		skillratio += -100 + 800 + 700 * skill_lv;
		if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
			skillratio += sc->getSCE(SC_INTENSIVE_AIM_COUNT)->val1 * 200 * skill_lv;
	}
	skillratio += 5 * sstatus->con;
	RE_LVL_DMOD(100);
}
