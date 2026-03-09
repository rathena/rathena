// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "chulhobattering.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillChulhoBattering::SkillChulhoBattering() : SkillImplRecursiveDamageSplash(SH_CHUL_HO_BATTERING) {
}

void SkillChulhoBattering::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 480 + 160 * skill_lv;
	skillratio += 70 * pc_checkskill(sd, SH_MYSTICAL_CREATURE_MASTERY);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillChulhoBattering::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
