// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "counterslash.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillCounterSlash::SkillCounterSlash() : SkillImplRecursiveDamageSplash(GC_COUNTERSLASH) {
}

void SkillCounterSlash::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	//ATK [{(Skill Level x 150) + 300} x Caster's Base Level / 120]% + ATK [(AGI x 2) + (Caster's Job Level x 4)]%
	skillratio += -100 + 300 + 150 * skill_lv;
	RE_LVL_DMOD(120);
	skillratio += sstatus->agi * 2;
	// If 4th job, job level of your 3rd job counts
	skillratio += (sd ? (sd->class_&JOBL_FOURTH ? sd->change_level_4th : sd->status.job_level) * 4 : 0);
}

void SkillCounterSlash::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
