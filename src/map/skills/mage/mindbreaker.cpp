// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mindbreaker.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillMindBreaker::SkillMindBreaker() : SkillImpl(PF_MINDBREAKER) {
}

void SkillMindBreaker::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	const status_data* tstatus = status_get_status_data(*target);
	status_change* tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	status_change_entry *tsce = (tsc != nullptr && type != SC_NONE) ? tsc->getSCE(type) : nullptr;
	map_session_data* sd = BL_CAST(BL_PC, src);
	mob_data* dstmd = BL_CAST(BL_MOB, target);

	if (status_has_mode(tstatus, MD_STATUSIMMUNE) || battle_check_undead(tstatus->race, tstatus->def_ele)) {
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	if (tsce != nullptr)
	{	//HelloKitty2 (?) explained that this silently fails when target is
		//already inflicted. [Skotlex]
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	//Has a 55% + skill_lv*5% success chance.
	if (!clif_skill_nodamage(src, *target, getSkillId(), skill_lv,
			sc_start(src, target, type, 55 + 5 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv)))) {
		if (sd != nullptr) {
			clif_skill_fail(*sd, getSkillId());
		}
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	unit_skillcastcancel(target, 0);

	if (dstmd != nullptr) {
		mob_target(dstmd, src, skill_get_range2(src, getSkillId(), skill_lv, true));
	}
}
