// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "destructivehurricane.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

// AG_DESTRUCTIVE_HURRICANE
SkillDestructiveHurricane::SkillDestructiveHurricane() : SkillImplRecursiveDamageSplash(AG_DESTRUCTIVE_HURRICANE) {
}

void SkillDestructiveHurricane::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 600 + 2850 * skill_lv + 5 * sstatus->spl;
	// (climax buff applied with pc_skillatk_bonus)
	RE_LVL_DMOD(100);
}

void SkillDestructiveHurricane::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	sc_type type = skill_get_sc(getSkillId());

	if (flag&1) { // Buff from Crystal Impact with level 1 Climax.
		sc_start(src, target, type, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
	} else {
		uint16 climax_lv = 0, splash_size = skill_get_splash(getSkillId(), skill_lv);

		if (sc && sc->getSCE(SC_CLIMAX))
			climax_lv = sc->getSCE(SC_CLIMAX)->val1;

		if (climax_lv == 5) { // Adjusts splash AoE size depending on skill.
			splash_size = 9; // 19x19
		}

		skill_area_temp[1] = 0;
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

		if (climax_lv == 4) // Buff for caster instead of damage AoE.
			sc_start(src, target, type, 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
		else {
			if (climax_lv == 1) // Display extra animation for the additional hit cast.
				clif_skill_nodamage(src, *target, AG_DESTRUCTIVE_HURRICANE_CLIMAX, skill_lv);

			map_foreachinrange(skill_area_sub, target, splash_size, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		}
	}
}

void SkillDestructiveHurricane::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change *sc = status_get_sc(src);

	// Targets hit are dealt a additional hit through Climax.
	if (sc && sc->getSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 1)
		skill_castend_damage_id(src, target, AG_DESTRUCTIVE_HURRICANE_CLIMAX, skill_lv, tick, SD_LEVEL|SD_ANIMATION);
}


// AG_DESTRUCTIVE_HURRICANE_CLIMAX
SkillDestructiveHurricaneClimax::SkillDestructiveHurricaneClimax() : SkillImpl(AG_DESTRUCTIVE_HURRICANE_CLIMAX) {
}

void SkillDestructiveHurricaneClimax::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 12500;
	// Skill not affected by Baselevel and SPL
}

void SkillDestructiveHurricaneClimax::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}
