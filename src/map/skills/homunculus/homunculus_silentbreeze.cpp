// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_silentbreeze.hpp"

#include <config/core.hpp>

#include "map/homunculus.hpp"
#include "map/status.hpp"

SkillSilentBreeze::SkillSilentBreeze() : SkillImpl(MH_SILENT_BREEZE) {
}

void SkillSilentBreeze::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	homun_data* hd = BL_CAST(BL_HOM, src);
	status_change* tsc = status_get_sc(target);
	int32 i = 0;
	int32 heal = 5 * status_get_lv(hd) +
#ifdef RENEWAL
		status_base_matk_min(target, &hd->battle_status, status_get_lv(hd));
#else
		status_base_matk_min(&hd->battle_status);
#endif
	//Silences the homunculus and target
	status_change_start(src, src, SC_SILENCE, 10000, skill_lv, 0, 0, 0, skill_get_time(getSkillId(), skill_lv), SCSTART_NONE);
	status_change_start(src, target, SC_SILENCE, 10000, skill_lv, 0, 0, 0, skill_get_time(getSkillId(), skill_lv), SCSTART_NONE);

	//Recover the target's HP
	status_heal(target, heal, 0, 3);

	//Removes these SC from target
	if (tsc) {
		const enum sc_type scs[] = {
			SC_MANDRAGORA, SC_HARMONIZE, SC_DEEPSLEEP, SC_VOICEOFSIREN, SC_SLEEP, SC_CONFUSION, SC_HALLUCINATION
		};
		for (i = 0; i < ARRAYLENGTH(scs); i++) {
			if (tsc->getSCE(scs[i])) {
				status_change_end(target, scs[i]);
			}
		}
	}
}
