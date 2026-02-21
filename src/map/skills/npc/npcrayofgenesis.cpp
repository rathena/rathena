// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcrayofgenesis.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/status.hpp"

SkillNpcRayOfGenesis::SkillNpcRayOfGenesis() : SkillImplRecursiveDamageSplash(NPC_RAYOFGENESIS) {
}

void SkillNpcRayOfGenesis::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	if (skill_lv < 8)
		sc_start(src,target, SC_BLIND, 50, skill_lv, skill_get_time(getSkillId(),skill_lv));
	else
		sc_start(src,target, SC_BLIND, 100, skill_lv, skill_get_time(getSkillId(),skill_lv));
}

void SkillNpcRayOfGenesis::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 200 * skill_lv;
}

void SkillNpcRayOfGenesis::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
