// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ragingpalmstrike.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillRagingPalmStrike::SkillRagingPalmStrike() : SkillImpl(CH_PALMSTRIKE) {
}

void SkillRagingPalmStrike::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	//	Palm Strike takes effect 1sec after casting. [Skotlex]
	// clif_skill_nodamage(src,*target,getSkillId(),skill_lv,false); //Can't make this one display the correct attack animation delay :/
	clif_damage(*src, *target, tick, status_get_amotion(src), 0, -1, 1, DMG_ENDURE, 0, false); //Display an absorbed damage attack.
	skill_addtimerskill(src, tick + (1000 + status_get_amotion(src)), target->id, 0, 0, getSkillId(), skill_lv, BF_WEAPON, flag);
}

void SkillRagingPalmStrike::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
#ifdef RENEWAL
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += 100 + 100 * skill_lv + sstatus->str; // !TODO: How does STR play a role?
	RE_LVL_DMOD(100);
#else
	skillratio += 100 + 100 * skill_lv;
#endif
}
