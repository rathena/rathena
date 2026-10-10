// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "terraburst.hpp"

#include "map/elemental.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillTerraBurst::SkillTerraBurst() : SkillImplRecursiveDamageSplash(EM_TERRA_BURST) {
}

void SkillTerraBurst::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);
	bool superior_spirit = sd != nullptr && sd->ed != nullptr && sd->ed->elemental.class_ == ELEMENTALID_TERREMOTUS;

	skillratio += -100 + 400 * skill_lv + (superior_spirit ? 5100 : 4400);
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}
