// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "thirdpunish.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillThirdPunish::SkillThirdPunish() : SkillImplRecursiveDamageSplash(IQ_THIRD_PUNISH) {
}

void SkillThirdPunish::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 450 + 1800 * skill_lv;
	skillratio += 10 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillThirdPunish::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change_end(target, SC_SECOND_BRAND);
}

void SkillThirdPunish::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	if (sd) {
		uint8 limit = 5;
		status_change* sc = status_get_sc(src);

		if (sc && sc->getSCE(SC_RAISINGDRAGON))
			limit += sc->getSCE(SC_RAISINGDRAGON)->val1;
		for (uint8 i = 0; i < limit; i++)
			pc_addspiritball(sd, skill_get_time(getSkillId(), skill_lv), limit);
	}

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
