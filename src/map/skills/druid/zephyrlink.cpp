// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "zephyrlink.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillZephyrLink::SkillZephyrLink() : SkillImplRecursiveDamageSplash(AT_ZEPHYR_LINK) {
}

void SkillZephyrLink::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
	sc_start(src, target, SC_ZEPHYR_LINK, 100, 0, skill_get_time(getSkillId(), skill_lv));
}
