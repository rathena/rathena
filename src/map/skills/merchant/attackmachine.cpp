// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "attackmachine.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillAttackMachine::SkillAttackMachine() : SkillImplRecursiveDamageSplash(MT_A_MACHINE) {
}

void SkillAttackMachine::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data*  dstsd = BL_CAST(BL_PC, target);

	if (flag & 1) {
		skill_area_temp[1] = 0;

		if (sd && pc_issit(sd)) { // Force player to stand before attacking
			pc_setstand(sd, true);
			skill_sit(sd, false);
		}

		map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR | BL_SKILL, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_LEVEL | SD_SPLASH, skill_castend_damage_id);
	} else {
		if (dstsd) {
			int32 lv = abs( status_get_lv( src ) - status_get_lv( target ) );

			if (lv > battle_config.attack_machine_level_difference) {
				if (sd)
					clif_skill_fail( *sd, getSkillId() );

				flag |= SKILL_NOCONSUME_REQ;
				return;
			}
		}

		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
	}
}

void SkillAttackMachine::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	// Formula unknown. Using Dancing Knife's formula for now. [Rytech]
	skillratio += -100 + 200 * skill_lv + 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
