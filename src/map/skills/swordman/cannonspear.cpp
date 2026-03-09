// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cannonspear.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillCannonSpear::SkillCannonSpear() : SkillImplRecursiveDamageSplash(LG_CANNONSPEAR) {
}

void SkillCannonSpear::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_area_temp[1] = 0;
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	int32 hits = map_foreachinrange(
		skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR | BL_SKILL,
		src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);

	if (!hits) {
		clif_skill_damage(*src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE);
	}
}

void SkillCannonSpear::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);

	skillratio += -100 + skill_lv * (120 + sstatus->str);

	if (sc != nullptr && sc->getSCE(SC_SPEAR_SCAR)) {
		skillratio += 400;
	}

	RE_LVL_DMOD(100);
}
