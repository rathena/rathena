// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kunaiexplosion.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillKunaiExplosion::SkillKunaiExplosion() : SkillImplRecursiveDamageSplash(KO_BAKURETSU) {
}

void SkillKunaiExplosion::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + (sd ? pc_checkskill(sd,NJ_TOBIDOUGU) : 1) * (50 + sstatus->dex / 4) * skill_lv * 4 / 10;
	RE_LVL_DMOD(120);
	skillratio += 10 * (sd ? sd->status.job_level : 1);
	if (sc && sc->getSCE(SC_KAGEMUSYA))
		skillratio += skillratio * sc->getSCE(SC_KAGEMUSYA)->val2 / 100;
}
