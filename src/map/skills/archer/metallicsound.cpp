// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "metallicsound.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillMetallicSound::SkillMetallicSound() : SkillImpl(WM_METALICSOUND) {
}

void SkillMetallicSound::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change_end(target, SC_SOUNDBLEND);
}

void SkillMetallicSound::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *tsc = status_get_sc(target);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 120 * skill_lv + 60 * ((sd) ? pc_checkskill(sd, WM_LESSON) : 1);
	if (tsc && tsc->getSCE(SC_SLEEP))
		skillratio += 100; // !TODO: Confirm target sleeping bonus
	RE_LVL_DMOD(100);
	if (tsc && tsc->getSCE(SC_SOUNDBLEND))
		skillratio += skillratio * 50 / 100;
}

void SkillMetallicSound::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}
