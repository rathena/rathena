// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sightlessmind.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSightlessMind::SkillSightlessMind() : SkillImplRecursiveDamageSplash(RG_RAID) {
}

void SkillSightlessMind::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += -100 + 50 + skill_lv * 150;
#else
	base_skillratio += 40 * skill_lv;
#endif
}

void SkillSightlessMind::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target,
		skill_get_splash(getSkillId(), skill_lv), BL_CHAR|BL_SKILL,
		src,getSkillId(),skill_lv,tick, flag|BCT_ENEMY|1,
		skill_castend_damage_id);
	status_change_end(src, SC_HIDING);
}

void SkillSightlessMind::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_STUN,(10+3*skill_lv),skill_lv,skill_get_time(getSkillId(),skill_lv));
	sc_start(src,target,SC_BLIND,(10+3*skill_lv),skill_lv,skill_get_time2(getSkillId(),skill_lv));
#ifdef RENEWAL
	sc_start(src, target, SC_RAID, 100, skill_lv, 10000); // Hardcoded to 10 seconds since Duration1 and Duration2 are used
#endif
}
