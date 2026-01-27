// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "souldestroyer.hpp"

#include <config/const.hpp>
#include <config/core.hpp>

#include "map/status.hpp"

SkillSoulDestroyer::SkillSoulDestroyer() : WeaponSkillImpl(ASC_BREAKER) {
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
