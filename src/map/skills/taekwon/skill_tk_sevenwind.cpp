// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_tk_sevenwind.hpp"

SkillSevenwind::SkillSevenwind() : SkillImpl(TK_SEVENWIND) {
}

void SkillSevenwind::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const {
	enum sc_type type = SC_NONE;

	switch (skill_get_ele(this->skill_id, skill_lv)) {
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

	clif_skill_nodamage(src, *bl, this->skill_id, skill_lv,
	                    sc_start(src, bl, type, 100, skill_lv, skill_get_time(this->skill_id, skill_lv)));

	sc_start(src, bl, SC_SEVENWIND, 100, skill_lv, skill_get_time(this->skill_id, skill_lv));
}
