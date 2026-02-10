// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "shadowstab.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillShadowStab::SkillShadowStab() : WeaponSkillImpl(SHC_SHADOW_STAB) {
}

void SkillShadowStab::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 550 * skill_lv;
	skillratio += 5 * sstatus->pow;

	if (wd->miscflag & SKILL_ALTDMG_FLAG) {
		skillratio += 100 * skill_lv + 2 * sstatus->pow;
	}

	RE_LVL_DMOD(100);
}

void SkillShadowStab::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);

	if (sc && sc->getSCE(SC_CLOAKINGEXCEED))
		flag |= SKILL_ALTDMG_FLAG;

	status_change_end(src, SC_CLOAKING);
	status_change_end(src, SC_CLOAKINGEXCEED);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
