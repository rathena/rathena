// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pinpointattack.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillPinpointAttack::SkillPinpointAttack() : WeaponSkillImpl(LG_PINPOINTATTACK) {
}

void SkillPinpointAttack::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (skill_check_unit_movepos(5, src, target->x, target->y, 1, 1)) {
		clif_blown(src);
	}

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillPinpointAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 100 * skill_lv + 5 * status_get_agi(src);
	RE_LVL_DMOD(120);
}

void SkillPinpointAttack::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	int32 rate = 30 + 5 * ((sd) ? pc_checkskill(sd, getSkillId()) : skill_lv) + (status_get_agi(src) + status_get_lv(src)) / 10;

	switch (skill_lv) {
		case 1:
			sc_start2(src, target, SC_BLEEDING, rate, skill_lv, src->id, skill_get_time(getSkillId(), skill_lv));
			break;
		case 2:
			skill_break_equip(src, target, EQP_HELM, rate * 100, BCT_ENEMY);
			break;
		case 3:
			skill_break_equip(src, target, EQP_SHIELD, rate * 100, BCT_ENEMY);
			break;
		case 4:
			skill_break_equip(src, target, EQP_ARMOR, rate * 100, BCT_ENEMY);
			break;
		case 5:
			skill_break_equip(src, target, EQP_WEAPON, rate * 100, BCT_ENEMY);
			break;
	}
}
