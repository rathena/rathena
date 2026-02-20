// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "phantomthrust.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillPhantomThrust::SkillPhantomThrust() : WeaponSkillImpl(RK_PHANTOMTHRUST) {
}

void SkillPhantomThrust::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	// ATK = [{(Skill Level x 50) + (Spear Master Level x 10)} x Caster's Base Level / 150] %
	skillratio += -100 + 50 * skill_lv + 10 * (sd ? pc_checkskill(sd,KN_SPEARMASTERY) : 5);
	RE_LVL_DMOD(150); // Base level bonus.
}

void SkillPhantomThrust::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	unit_setdir(src,map_calc_dir(src, target->x, target->y));
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	skill_blown(src,target,distance_bl(src,target)-1,unit_getdir(src),BLOWN_NONE);
	if( battle_check_target(src,target,BCT_ENEMY) > 0 )
		WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
