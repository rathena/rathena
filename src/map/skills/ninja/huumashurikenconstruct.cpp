// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "huumashurikenconstruct.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHuumaShurikenConstruct::SkillHuumaShurikenConstruct() : WeaponSkillImpl(SS_FUUMAKOUCHIKU) {
}

void SkillHuumaShurikenConstruct::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 900 + 1750 * skill_lv;
	if( wd->miscflag&SKILL_ALTDMG_FLAG ){
		skillratio += 200;
	}
	skillratio += pc_checkskill( sd, SS_FUUMASHOUAKU ) * 100 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillHuumaShurikenConstruct::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_area_temp[1] = 0;
	if (battle_config.skill_eightpath_algorithm) {
		//Use official AoE algorithm
		map_foreachindir(skill_attack_area, src->m, src->x, src->y, x, y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_maxcount(getSkillId(), skill_lv), 0, BL_CHAR | BL_SKILL,
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	}
	else {
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, x, y,
			skill_get_splash(getSkillId(), skill_lv), skill_get_maxcount(getSkillId(), skill_lv), BL_CHAR | BL_SKILL,
			skill_get_type(getSkillId()), src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
	}
}
