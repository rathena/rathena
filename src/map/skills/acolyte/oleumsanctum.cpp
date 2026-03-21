// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "oleumsanctum.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillOleumSanctum::SkillOleumSanctum() : SkillImplRecursiveDamageSplash(IQ_OLEUM_SANCTUM) {
}

void SkillOleumSanctum::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	skill_castend_damage_id(src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillOleumSanctum::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 500 + 2000 * skill_lv + 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillOleumSanctum::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_HOLY_OIL, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
