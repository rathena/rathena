// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "alphaphase.hpp"

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillAlphaPhase::SkillAlphaPhase() : SkillImpl(AT_ALPHA_PHASE) {
}

void SkillAlphaPhase::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 reduce = 0;
	int32 hp_rate = 5 * skill_lv;
	int32 sp_rate = 3 * skill_lv;

	switch (skill_lv) {
	case 1:
		reduce = 60;
		break;
	case 2:
		reduce = 70;
		break;
	case 3:
		reduce = 80;
		break;
	case 4:
		reduce = 90;
		break;
	default:
		reduce = 99;
		break;
	}

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	sc_start(src, target, SC_NATURE_PROTECTION, 100, reduce, skill_get_time(getSkillId(), skill_lv));

	int64 heal_hp = static_cast<int64>(status_get_max_hp(target)) * hp_rate / 100;
	int64 heal_sp = static_cast<int64>(status_get_max_sp(target)) * sp_rate / 100;
	if (heal_hp > 0 || heal_sp > 0) {
		status_heal(target, heal_hp, heal_sp, 0, 2);
	}
}
