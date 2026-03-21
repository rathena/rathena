// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shieldshooting.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillShieldShooting::SkillShieldShooting() : SkillImplRecursiveDamageSplash(IG_SHIELD_SHOOTING) {
}

void SkillShieldShooting::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1000 + 3500 * skill_lv;
	skillratio += 10 * sstatus->pow;
	skillratio += skill_lv * 150 * pc_checkskill(sd, IG_SHIELD_MASTERY);
	if (sd) { // Damage affected by the shield's weight and refine. Need official formula. [Rytech]
		int16 index = sd->equip_index[EQI_HAND_L];

		if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_ARMOR) {
			skillratio += (sd->inventory_data[index]->weight * 7 / 6) / 10;
			skillratio += sd->inventory.u.items_inventory[index].refine * 100;
		}
	}
	RE_LVL_DMOD(100);
}

void SkillShieldShooting::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
