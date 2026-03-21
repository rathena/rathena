// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sightrasher.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSightRasher::SkillSightRasher() : SkillImpl(WZ_SIGHTRASHER) {
}

void SkillSightRasher::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	//Passive side of the attack.
	status_change_end(src, SC_SIGHT);
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinshootrange(skill_area_sub,src,
		skill_get_splash(getSkillId(), skill_lv),BL_CHAR|BL_SKILL,
		src,getSkillId(),skill_lv,tick, flag|BCT_ENEMY|SD_ANIMATION|1,
		skill_castend_damage_id);
}

void SkillSightRasher::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}

void SkillSightRasher::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	base_skillratio += 20 * skill_lv;
}
