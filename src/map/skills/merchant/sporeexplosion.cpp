// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sporeexplosion.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillSporeExplosion::SkillSporeExplosion() : SkillImplRecursiveDamageSplash(GN_SPORE_EXPLOSION) {
}

void SkillSporeExplosion::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_SPORE_EXPLOSION, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillSporeExplosion::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	skillratio += -100 + 400 + 200 * skill_lv;
	RE_LVL_DMOD(100);
	if (sc && sc->getSCE(SC_BIONIC_WOODEN_FAIRY))
		skillratio *= 2;
}
