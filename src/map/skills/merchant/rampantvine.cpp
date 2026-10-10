// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rampantvine.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillRampantVine::SkillRampantVine() : WeaponSkillImpl(BO_RAMPANT_VINE) {
}

void SkillRampantVine::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 700 * skill_lv + 1000 + 40 * pc_checkskill(sd, BO_BIONICS_M);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillRampantVine::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(&src);

	dmg.div_ = (sc != nullptr && sc->hasSCE(SC_BIONIC_CREEPER)) ? 5 : 2;
}
