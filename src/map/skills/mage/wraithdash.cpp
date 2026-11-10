// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "wraithdash.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillWraithDash::SkillWraithDash() : SkillImplRecursiveDamageSplash(AG_WRAITH_DASH) {
}

void SkillWraithDash::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	// TODO: Confirm whether the official formula includes SPL or Base Level scaling.
	skillratio += -100 + 4000 * skill_lv + 3000;
}

void SkillWraithDash::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	// The database knockback distance controls the caster's dash only.
	dmg.blewcount = 0;
}

void SkillWraithDash::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	uint8 dir = (unit_getdir(src) + 4) % 8;

	skill_blown(src, target, skill_get_blewcount(getSkillId(), skill_lv), dir, BLOWN_IGNORE_NO_KNOCKBACK);
}
