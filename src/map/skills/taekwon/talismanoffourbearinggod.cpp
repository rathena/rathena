// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "talismanoffourbearinggod.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTalismanOfFourBearingGod::SkillTalismanOfFourBearingGod() : SkillImplRecursiveDamageSplash(SOA_TALISMAN_OF_FOUR_BEARING_GOD) {
}

void SkillTalismanOfFourBearingGod::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change *sc = status_get_sc(&src);

	if (sc != nullptr){
		if (sc->hasSCE(SC_T_FIRST_GOD))
			dmg.div_ = 2;
		else if (sc->hasSCE(SC_T_SECOND_GOD))
			dmg.div_ = 3;
		else if (sc->hasSCE(SC_T_THIRD_GOD))
			dmg.div_ = 4;
		else if (sc->hasSCE(SC_T_FOURTH_GOD))
			dmg.div_ = 5;
		else if (sc->hasSCE(SC_T_FIFTH_GOD))
			dmg.div_ = 7;
	}
}

void SkillTalismanOfFourBearingGod::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 50 + 250 * skill_lv;
	skillratio += pc_checkskill(sd, SOA_TALISMAN_MASTERY) * 15 * skill_lv;
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillTalismanOfFourBearingGod::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
