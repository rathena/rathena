// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "overbrand.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillOverBrand::SkillOverBrand() : SkillImplRecursiveDamageSplash(LG_OVERBRAND) {
}

void SkillOverBrand::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}

void SkillOverBrand::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);

	if(sc && sc->getSCE(SC_OVERBRANDREADY))
		skillratio += -100 + 500 * skill_lv;
	else
		skillratio += -100 + 350 * skill_lv;
	skillratio += ((sd) ? pc_checkskill(sd, CR_SPEARQUICKEN) * 50 : 0);
	RE_LVL_DMOD(100);
}
