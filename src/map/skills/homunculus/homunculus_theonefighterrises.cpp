// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_theonefighterrises.hpp"

#include "map/clif.hpp"
#include "map/homunculus.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTheOneFighterRises::SkillTheOneFighterRises() : SkillImplRecursiveDamageSplash(MH_THE_ONE_FIGHTER_RISES) {
}

void SkillTheOneFighterRises::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	homun_data* hd = BL_CAST(BL_HOM, src);
	int32 starget = BL_CHAR|BL_SKILL;

	hom_addspiritball(hd, MAX_SPIRITBALL);

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
				src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}

void SkillTheOneFighterRises::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 580 * skill_lv * status_get_lv(src) / 100 + sstatus->str;
}
