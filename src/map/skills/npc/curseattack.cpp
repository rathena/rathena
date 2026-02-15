// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "curseattack.hpp"

SkillCurseAttack::SkillCurseAttack() : WeaponSkillImpl(NPC_CURSEATTACK) {
}

void SkillCurseAttack::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	nullpo_retv(src);
	nullpo_retv(target);
	(void)skill_lv;
	hit_rate += hit_rate * 20 / 100;
}

void SkillCurseAttack::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	(void)tick;
	(void)attack_type;
	(void)dmg_lv;
	sc_start(src, target, SC_CURSE, (20 * skill_lv), skill_lv, skill_get_time2(getSkillId(), skill_lv));
}
