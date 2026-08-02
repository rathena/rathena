// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_toxinofmandara.hpp"

#include "map/status.hpp"

SkillToxinOfMandara::SkillToxinOfMandara() : SkillImplRecursiveDamageSplash(MH_TOXIN_OF_MANDARA) {
}

void SkillToxinOfMandara::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 400 + 450 * skill_lv * status_get_lv(src) / 100 + sstatus->dex; // !TODO: Confirm Base Level and DEX bonus
}

void SkillToxinOfMandara::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_TOXIN_OF_MANDARA, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
