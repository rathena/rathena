// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "servantweaponcleave.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillServantWeaponCleave::SkillServantWeaponCleave() : WeaponSkillImpl(DK_SERVANT_W_CLEAVE) {
}

void SkillServantWeaponCleave::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);

	if (sc != nullptr && sc->hasSCE(SC_VIGOR))
		skillratio += -100 + 1050 * skill_lv + 150;
	else
		skillratio += -100 + 750 * skill_lv + 300;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillServantWeaponCleave::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(&src);

	dmg.div_ = (sc != nullptr && sc->hasSCE(SC_VIGOR)) ? 4 : 3;
}

void SkillServantWeaponCleave::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	// Consumes one servant weapon (checked before casting).
	if (sd != nullptr && sd->servantball > 0)
		pc_delservantball(*sd, 1);

	WeaponSkillImpl::castendDamageId(src, target, skill_lv, tick, flag);
}
