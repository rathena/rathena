// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rayofgenesis.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillRayOfGenesis::SkillRayOfGenesis() : SkillImplRecursiveDamageSplash(LG_RAYOFGENESIS) {
}

void SkillRayOfGenesis::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}

void SkillRayOfGenesis::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 350 * skill_lv;
	skillratio += sstatus->int_ * 3;
	RE_LVL_DMOD(100);
}

void SkillRayOfGenesis::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_data* tstatus = status_get_status_data(*target);

	// 50% chance to cause Blind on Undead and Demon monsters.
	if ( battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON )
		sc_start(src,target, SC_BLIND, 50, skill_lv, skill_get_time(getSkillId(),skill_lv));
}
