// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "glacierfist.hpp"

#include <config/core.hpp>

#include "map/status.hpp"

SkillGlacierFist::SkillGlacierFist() : WeaponSkillImpl(CH_TIGERFIST) {
}

void SkillGlacierFist::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
#ifdef RENEWAL
	skillratio += 400 + 150 * skill_lv;
	RE_LVL_DMOD(100);
#else
	skillratio += -60 + 100 * skill_lv;
#endif
	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->getSCE(SC_GT_ENERGYGAIN))
		skillratio += skillratio * 50 / 100;
}

void SkillGlacierFist::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	t_tick basetime = skill_get_time(getSkillId(), skill_lv);
	t_tick mintime = 15 * (status_get_lv(src) + 100);

	if (status_bl_has_mode(target, MD_STATUSIMMUNE))
		basetime /= 5;
	basetime = std::max((basetime * status_get_agi(target)) / -200 + basetime, mintime);
	sc_start(src, target, SC_ANKLE, (1 + skill_lv) * 10, 0, basetime);
}
