// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "windcutter.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWindCutter::SkillWindCutter() : SkillImpl(RK_WINDCUTTER) {
}

void SkillWindCutter::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		if (sd->weapontype1 == W_2HSWORD)
			skillratio += -100 + 250 * skill_lv;
		else if (sd->weapontype1 == W_1HSPEAR || sd->weapontype1 == W_2HSPEAR)
			skillratio += -100 + 400 * skill_lv;
		else
			skillratio += -100 + 300 * skill_lv;
	} else
		skillratio += -100 + 300 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillWindCutter::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if( flag&1 )
		skill_attack(skill_get_type(getSkillId()),src,src,target,getSkillId(),skill_lv,tick,flag);
	else {
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		map_foreachinallrange(skill_area_sub, target,skill_get_splash(getSkillId(), skill_lv),BL_CHAR,src,getSkillId(),skill_lv,tick, flag|BCT_ENEMY|1,skill_castend_nodamage_id);
	}
}

void SkillWindCutter::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 starget = BL_CHAR|BL_SKILL;

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	int32 i = map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
			src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
	if( !i )
		clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
}
