// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crystalimpact.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

// AG_CRYSTAL_IMPACT
SkillCrystalImpact::SkillCrystalImpact() : SkillImplRecursiveDamageSplash(AG_CRYSTAL_IMPACT) {
}

void SkillCrystalImpact::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 250 + 1300 * skill_lv + 5 * sstatus->spl;
	// (climax buff applied with pc_skillatk_bonus)
	RE_LVL_DMOD(100);
}

void SkillCrystalImpact::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	sc_type type = skill_get_sc(getSkillId());

	if (flag&1) { // Buff from Crystal Impact with level 1 Climax.
		sc_start(src, target, type, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	} else {
		uint16 climax_lv = 0, splash_size = skill_get_splash(getSkillId(), skill_lv);

		if (sc && sc->getSCE(SC_CLIMAX))
			climax_lv = sc->getSCE(SC_CLIMAX)->val1;

		if (climax_lv == 5) { // Adjusts splash AoE size depending on skill.
			splash_size = 7; // 15x15
		}

		skill_area_temp[1] = 0;
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

		if (climax_lv == 1) // Buffs the caster and allies instead of doing damage AoE.
			map_foreachinrange(skill_area_sub, target, splash_size, BL_CHAR, src, getSkillId(), skill_lv, tick, flag|BCT_ALLY|SD_SPLASH|1, skill_castend_nodamage_id);
		else
			map_foreachinrange(skill_area_sub, target, splash_size, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
	}
}

void SkillCrystalImpact::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Targets hit are dealt aftershock damage.
	skill_castend_damage_id(src, target, AG_CRYSTAL_IMPACT_ATK, skill_lv, tick, SD_LEVEL);
}


// AG_CRYSTAL_IMPACT_ATK
SkillCrystalImpactAttack::SkillCrystalImpactAttack() : SkillImplRecursiveDamageSplash(AG_CRYSTAL_IMPACT_ATK) {
}

void SkillCrystalImpactAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 250 + 1300 * skill_lv + 5 * sstatus->spl;
	// (climax buff applied with pc_skillatk_bonus)
	RE_LVL_DMOD(100);
}

int16 SkillCrystalImpactAttack::getSearchSize(block_list* src, uint16 skill_lv) const {
	status_change *sc = status_get_sc(src);

	if (sc != nullptr && sc->hasSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 5)
		return 2;// Gives the aftershock hit a 5x5 splash AoE.

	return SkillImplRecursiveDamageSplash::getSearchSize(src, skill_lv);
}

int16 SkillCrystalImpactAttack::getSplashSearchSize(block_list* src, uint16 skill_lv) const {
	status_change *sc = status_get_sc(src);

	if (sc != nullptr && sc->hasSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 5)
		return 2;// Gives the aftershock hit a 5x5 splash AoE.

	return SkillImplRecursiveDamageSplash::getSplashSearchSize(src, skill_lv);
}
