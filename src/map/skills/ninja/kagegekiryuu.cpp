// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kagegekiryuu.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillKagegekiryuu::SkillKagegekiryuu() : SkillImplRecursiveDamageSplash(SS_KAGEGEKIRYU) {
}

void SkillKagegekiryuu::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + ((sc != nullptr && sc->hasSCE(SC_NOBORU)) ? 850 : 650) * skill_lv + 120 * pc_checkskill(sd, SS_KAGENOMAI);
}
