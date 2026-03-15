// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chainreactionshot.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillChainReactionShot::SkillChainReactionShot() : SkillImplRecursiveDamageSplash(ABC_CHAIN_REACTION_SHOT) {
}

void SkillChainReactionShot::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 850 * skill_lv;
	skillratio += 15 * sstatus->con;
	RE_LVL_DMOD(100);
}

void SkillChainReactionShot::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(ABC_CHAIN_REACTION_SHOT_ATK, skill_lv), BL_CHAR | BL_SKILL, src, ABC_CHAIN_REACTION_SHOT_ATK, skill_lv, tick + (200 + status_get_amotion(src)), flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

SkillChainReactionShotAttack::SkillChainReactionShotAttack() : WeaponSkillImpl(ABC_CHAIN_REACTION_SHOT_ATK) {
}

void SkillChainReactionShotAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 800 + 2550 * skill_lv;
	skillratio += 15 * sstatus->con;
	if (sc != nullptr && sc->hasSCE(SC_CHASING))
		skillratio += 700 * skill_lv;
	RE_LVL_DMOD(100);
}
