// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "servantweapon.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillServantWeapon::SkillServantWeapon() : SkillImpl(DK_SERVANTWEAPON) {
}

void SkillServantWeapon::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv, sc_start2(src, target, skill_get_sc(getSkillId()), 100, skill_lv, src->id, skill_get_time(getSkillId(), skill_lv)));
}

SkillServantWeaponAttack::SkillServantWeaponAttack() : SkillImplRecursiveDamageSplash(DK_SERVANTWEAPON_ATK) {
}

void SkillServantWeaponAttack::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 600 + 850 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
