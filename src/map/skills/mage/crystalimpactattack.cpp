// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crystalimpactattack.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillCrystalImpactAttack::SkillCrystalImpactAttack() : SkillImplRecursiveDamageSplash(AG_CRYSTAL_IMPACT_ATK) {
}

void SkillCrystalImpactAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 250 + 1300 * skill_lv + 5 * sstatus->spl;
	// (climax buff applied with pc_skillatk_bonus)
	RE_LVL_DMOD(100);
}

void SkillCrystalImpactAttack::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	status_change *sc = status_get_sc(src);
	int32 splash_size;

	if (sc && sc->getSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 5)
		splash_size = 2;// Gives the aftershock hit a 5x5 splash AoE.
	else
		splash_size = SkillImplRecursiveDamageSplash::getSearchSize(skill_lv);

	// if skill damage should be split among targets, count them
	// SD_LEVEL -> Forced splash damage -> count targets
	if (flag & SD_LEVEL || skill_get_nk(getSkillId(), NK_SPLASHSPLIT)){
		skill_area_temp[0] = map_foreachinallrange(skill_area_sub, target, splash_size, BL_CHAR, src, getSkillId(), skill_lv, tick, BCT_ENEMY, skill_area_sub_count);
		// If there are no characters in the area, then it always counts as if there was one target
		// This happens when targetting skill units such as icewall
		skill_area_temp[0] = std::max(1, skill_area_temp[0]);
	}

	// recursive invocation of skill_castend_damage_id() with flag|1
	map_foreachinrange(skill_area_sub, target, splash_size, SkillImplRecursiveDamageSplash::getSplashTarget(src), src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
}
