// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_needleofparalyze.hpp"

#include "map/status.hpp"

SkillNeedleOfParalyze::SkillNeedleOfParalyze() : SkillImpl(MH_NEEDLE_OF_PARALYZE) {
}

void SkillNeedleOfParalyze::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillNeedleOfParalyze::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const status_data *sstatus = status_get_status_data(*src);

	base_skillratio += -100 + 450 * skill_lv * status_get_lv(src) / 100 + sstatus->dex; // !TODO: Confirm Base Level and DEX bonus
}

void SkillNeedleOfParalyze::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_PARALYSIS, 30 + 5 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
