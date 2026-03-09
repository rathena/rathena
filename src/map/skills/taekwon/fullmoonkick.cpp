// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fullmoonkick.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillFullMoonKick::SkillFullMoonKick() : SkillImplRecursiveDamageSplash(SJ_FULLMOONKICK) {
}

void SkillFullMoonKick::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_BLIND, 15 + 5 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillFullMoonKick::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	skillratio += 1000 + 100 * skill_lv;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_LIGHTOFMOON))
		skillratio += skillratio * sc->getSCE(SC_LIGHTOFMOON)->val2 / 100;
}

void SkillFullMoonKick::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
