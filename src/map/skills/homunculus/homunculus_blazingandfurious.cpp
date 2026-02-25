// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_blazingandfurious.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillBlazingAndFurious::SkillBlazingAndFurious() : SkillImplRecursiveDamageSplash(MH_BLAZING_AND_FURIOUS) {
}

void SkillBlazingAndFurious::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	/* Check if the target is an enemy; if not, skill should fail so the character doesn't unit_movepos (exploitable) */
	if( battle_check_target(src, target, BCT_ENEMY) > 0 ) {
		if( unit_movepos(src, target->x, target->y, 2, 1) ) {
			skill_attack(BF_WEAPON,src,src,target,getSkillId(),skill_lv,tick,flag);
			clif_blown(src);
		}
	}else if( sd ){
		clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
	}
}

void SkillBlazingAndFurious::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 80 * skill_lv * status_get_lv(src) / 100 + sstatus->str;
}
