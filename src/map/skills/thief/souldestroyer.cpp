// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "souldestroyer.hpp"

#include <config/const.hpp>
#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillSoulDestroyer::SkillSoulDestroyer() : WeaponSkillImpl(ASC_BREAKER) {
}

void SkillSoulDestroyer::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Calculate and apply damage without display
	const e_skill skill_id = getSkillId();
	int64 damage = skill_attack(BF_WEAPON, src, src, target, skill_id, skill_lv, tick, flag | SD_NODISPLAY);

	// Handle display ourselves - get damage type from skill_db
	status_data* sstatus = status_get_status_data(*src);
	status_data* tstatus = status_get_status_data(*target);
	clif_skill_damage(*src, *target, tick, sstatus->amotion, tstatus->dmotion, damage, skill_get_num(skill_id, skill_lv), skill_id, skill_lv, skill_get_hit(skill_id));
}

void SkillSoulDestroyer::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
#ifdef RENEWAL
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 150 * skill_lv + sstatus->str + sstatus->int_; // !TODO: Confirm stat modifier
	RE_LVL_DMOD(100);
#else
	// Pre-Renewal: skill ratio for weapon part of damage [helvetica]
	skillratio += -100 + 100 * skill_lv;
#endif
}
