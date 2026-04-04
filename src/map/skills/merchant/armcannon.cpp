// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "armcannon.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillArmCannon::SkillArmCannon() : SkillImplRecursiveDamageSplash(NC_ARMSCANNON) {
}

void SkillArmCannon::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change *sc = status_get_sc(&src);

	if (sc != nullptr && sc->hasSCE(SC_ABR_DUAL_CANNON))
		dmg.div_ = 2;
}

void SkillArmCannon::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 400 + 350 * skill_lv;
	RE_LVL_DMOD(100);
}

int32 SkillArmCannon::getSplashTarget(block_list* src) const {
	return splash_target(src);
}

void SkillArmCannon::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	skill_area_temp[1] = 0;

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

void SkillArmCannon::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
	const map_session_data* sd = BL_CAST(BL_PC, &src);

	if (sd != nullptr && sd->state.arrow_atk > 0)
		element = sd->bonus.arrow_ele;
}
