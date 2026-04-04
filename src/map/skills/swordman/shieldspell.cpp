// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shieldspell.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"
SkillShieldSpell::SkillShieldSpell() : SkillImpl(LG_SHIELDSPELL) {
}

void SkillShieldSpell::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type;

	if (skill_lv == 1) {
		type = SC_SHIELDSPELL_HP;
	} else if (skill_lv == 2) {
		type = SC_SHIELDSPELL_SP;
	} else {
		type = SC_SHIELDSPELL_ATK;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv,
		sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv)));
}
