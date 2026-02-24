// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "jackfrost2.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillJackFrost2::SkillJackFrost2() : SkillImplRecursiveDamageSplash(NPC_JACKFROST) {
}

void SkillJackFrost2::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_FREEZE,200,skill_lv,skill_get_time(getSkillId(),skill_lv));
}

void SkillJackFrost2::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *tsc = status_get_sc(target);

	if (tsc && tsc->getSCE(SC_FREEZING)) {
		skillratio += 900 + 300 * skill_lv;
		RE_LVL_DMOD(100);
	} else {
		skillratio += 400 + 100 * skill_lv;
		RE_LVL_DMOD(150);
	}
}

void SkillJackFrost2::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub,target,skill_get_splash(getSkillId(),skill_lv),BL_CHAR|BL_SKILL,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
}
