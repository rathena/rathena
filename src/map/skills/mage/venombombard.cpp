// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "venombombard.hpp"

#include "map/elemental.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillVenomBombard::SkillVenomBombard() : SkillImplRecursiveDamageSplash(EM_VENOM_BOMBARD) {
}

void SkillVenomBombard::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);
	bool superior_spirit = sd != nullptr && sd->ed != nullptr && sd->ed->elemental.class_ == ELEMENTALID_SERPENS;

	skillratio += -100 + 400 * skill_lv + (superior_spirit ? 5100 : 4400);
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}
