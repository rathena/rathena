// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "severerainstorm.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

// WM_SEVERE_RAINSTORM
SkillSevereRainstorm::SkillSevereRainstorm() : SkillImpl(WM_SEVERE_RAINSTORM) {
}

void SkillSevereRainstorm::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	flag |= 1;
	if (sd)
		sd->canequip_tick = tick + skill_get_time(getSkillId(), skill_lv); // Can't switch equips for the duration of the skill.
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}


// WM_SEVERE_RAINSTORM_MELEE
SkillSevereRainstormMelee::SkillSevereRainstormMelee() : WeaponSkillImpl(WM_SEVERE_RAINSTORM_MELEE) {
}

void SkillSevereRainstormMelee::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	//ATK [{(Caster DEX / 300 + AGI / 200)} x Caster Base Level / 100] %
	skillratio += -100 + 100 * skill_lv + (sstatus->dex / 300 + sstatus->agi / 200);
	if (wd->miscflag&4) // Whip/Instrument equipped
		skillratio += 20 * skill_lv;
	RE_LVL_DMOD(100);
}
