// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "effligo.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEffligo::SkillEffligo() : WeaponSkillImpl(CD_EFFLIGO) {
}

void SkillEffligo::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillEffligo::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 1650 * skill_lv + 7 * sstatus->pow;
	skillratio += 8 * pc_checkskill(sd, CD_MACE_BOOK_M);
	if (tstatus->race == RC_UNDEAD || tstatus->race == RC_DEMON) {
		skillratio += 150 * skill_lv;
		skillratio += 7 * pc_checkskill(sd, CD_MACE_BOOK_M);
	}
	RE_LVL_DMOD(100);
}
