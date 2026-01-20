// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "piercingshot.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillPiercingShot::SkillPiercingShot() : WeaponSkillImpl(GS_PIERCINGSHOT) {
}

void SkillPiercingShot::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd && sd->weapontype1 == W_RIFLE)
		base_skillratio += 150 + 30 * skill_lv;
	else
		base_skillratio += 100 + 20 * skill_lv;
#else
	base_skillratio += 20 * skill_lv;
#endif
}

void SkillPiercingShot::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start2(src, target, SC_BLEEDING, (skill_lv * 3), skill_lv, src->id, skill_get_time2(getSkillId(), skill_lv));
}
