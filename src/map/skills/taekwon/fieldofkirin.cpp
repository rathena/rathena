// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "fieldofkirin.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillFieldOfKirin::SkillFieldOfKirin() : SkillImplRecursiveDamageSplash(SOA_FIELD_OF_KIRIN) {
}

void SkillFieldOfKirin::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + ((sc != nullptr && sc->hasSCE(SC_T_FIFTH_GOD)) ? 1950 : 1550) * skill_lv + 850 + 100 * pc_checkskill(sd, SOA_TALISMAN_MASTERY);
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillFieldOfKirin::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
	// Only Mild Wind changes the neutral element specified by this skill.
	const status_change* sc = status_get_sc(&src);
	if (sc != nullptr && sc->hasSCE(SC_SEVENWIND))
		element = sc->getSCE(SC_SEVENWIND)->val1;
}

void SkillFieldOfKirin::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	// The client selects a target within 9 cells, but damage is centered on the caster.
	SkillImplRecursiveDamageSplash::splashSearch(src, src, skill_lv, tick, flag);
}
