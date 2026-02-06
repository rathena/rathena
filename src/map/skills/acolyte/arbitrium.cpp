// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "arbitrium.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillArbitrium::SkillArbitrium() : SkillImpl(CD_ARBITRIUM) {
}

void SkillArbitrium::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillArbitrium::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1000 * skill_lv + 10 * sstatus->spl;
	skillratio += 10 * pc_checkskill(sd, CD_FIDUS_ANIMUS) * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillArbitrium::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	// Target is Deep Silenced by chance and is then dealt a 2nd splash hit.
	sc_start(src, target, SC_HANDICAPSTATE_DEEPSILENCE, 20 + 5 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
	skill_castend_damage_id(src, target, CD_ARBITRIUM_ATK, skill_lv, tick, SD_LEVEL);
}

SkillArbitriumAttack::SkillArbitriumAttack() : SkillImplRecursiveDamageSplash(CD_ARBITRIUM_ATK) {
}

void SkillArbitriumAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1750 * skill_lv + 10 * sstatus->spl;
	skillratio += 50 * pc_checkskill(sd, CD_FIDUS_ANIMUS) * skill_lv;
	RE_LVL_DMOD(100);
}
