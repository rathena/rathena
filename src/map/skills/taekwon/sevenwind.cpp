// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sevenwind.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillSevenWind::SkillSevenWind() : SkillImpl(TK_SEVENWIND) {
}

void SkillSevenWind::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	sc_type type = SC_NONE;

	if (battle_config.buff_icons_tk == 0) { // [llchrisll]
		switch (skill_get_ele(getSkillId(), skill_lv)) {

		case ELE_EARTH:
			type = SC_EARTHWEAPON;
			break;
		case ELE_WIND:
			type = SC_WINDWEAPON;
			break;
		case ELE_WATER:
			type = SC_WATERWEAPON;
			break;
		case ELE_FIRE:
			type = SC_FIREWEAPON;
			break;
		case ELE_GHOST:
			type = SC_GHOSTWEAPON;
			break;
		case ELE_DARK:
			type = SC_SHADOWWEAPON;
			break;
		case ELE_HOLY:
			type = SC_ASPERSIO;
			break;
		}

	}
	else {
		switch (skill_get_ele(getSkillId(), skill_lv)) {

		case ELE_EARTH:
			type = SC_EARTHWEAPON_TK;
			break;
		case ELE_WIND:
			type = SC_WINDWEAPON_TK;
			break;
		case ELE_WATER:
			type = SC_WATERWEAPON_TK;
			break;
		case ELE_FIRE:
			type = SC_FIREWEAPON_TK;
			break;
		case ELE_GHOST:
			type = SC_GHOSTWEAPON;
			break;
		case ELE_DARK:
			type = SC_SHADOWWEAPON;
			break;
		case ELE_HOLY:
			type = SC_HOLYWEAPON_TK;
			break;
		}
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
	sc_start(src, target, SC_SEVENWIND, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
