// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "cloudkill.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillCloudKill::SkillCloudKill() : SkillImpl(SO_CLOUD_KILL) {
}

void SkillCloudKill::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time2(getSkillId(), skill_lv));
}

void SkillCloudKill::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 40 * skill_lv;
	skillratio += sstatus->int_ * 3;
	RE_LVL_DMOD(100);
	if (sc) {
		if (sc->getSCE(SC_CURSED_SOIL_OPTION))
			skillratio += (sd ? sd->status.job_level : 0);

		if (sc->getSCE(SC_DEEP_POISONING_OPTION))
			skillratio += skillratio * 1500 / 100;
	}
}

void SkillCloudKill::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag |= 4;
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
