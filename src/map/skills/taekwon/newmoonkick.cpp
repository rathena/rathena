// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "newmoonkick.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillNewMoonKick::SkillNewMoonKick() : SkillImplRecursiveDamageSplash(SJ_NEWMOONKICK) {
}

void SkillNewMoonKick::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 600 + 100 * skill_lv;
}

void SkillNewMoonKick::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(target);
	status_change_entry *tsce = (tsc != nullptr && type != SC_NONE) ? tsc->getSCE(type) : nullptr;

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	if (tsce) {
		status_change_end(target, type);
		return;
	} else
		sc_start(src, target, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));

	skill_castend_damage_id(src, target, getSkillId(), skill_lv, tick, flag);
}
