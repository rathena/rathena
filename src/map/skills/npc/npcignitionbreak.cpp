// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcignitionbreak.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/status.hpp"

SkillNpcIgnitionBreak::SkillNpcIgnitionBreak() : SkillImplRecursiveDamageSplash(NPC_IGNITIONBREAK) {
}

void SkillNpcIgnitionBreak::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	// 3x3 cell Damage   = 1000  1500  2000  2500  3000 %
	// 7x7 cell Damage   = 750   1250  1750  2250  2750 %
	// 11x11 cell Damage = 500   1000  1500  2000  2500 %
	int32 i = distance_bl(src,target);
	if (i < 2)
		base_skillratio += -100 + 500 * (skill_lv + 1);
	else if (i < 4)
		base_skillratio += -100 + 250 + 500 * skill_lv;
	else
		base_skillratio += -100 + 500 * skill_lv;
}

void SkillNpcIgnitionBreak::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_area_temp[1] = 0;
#if PACKETVER >= 20180207
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
#else
	clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
#endif
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR|BL_SKILL, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
