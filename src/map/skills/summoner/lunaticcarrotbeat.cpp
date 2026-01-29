// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lunaticcarrotbeat.hpp"
#include "lunaticcarrotbeat2.hpp"

#include <config/const.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillLunaticCarrotBeat::SkillLunaticCarrotBeat() : SkillImplRecursiveDamageSplash(SU_LUNATICCARROTBEAT) {
}

void SkillLunaticCarrotBeat::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += 100 + 100 * skill_lv;
	if (sd && pc_checkskill(sd, SU_SPIRITOFLIFE))
		skillratio += skillratio * status_get_hp(src) / status_get_max_hp(src);
	if (status_get_lv(src) > 99)
		skillratio += sstatus->str;
	RE_LVL_DMOD(100);
}

void SkillLunaticCarrotBeat::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd && pc_search_inventory(sd, skill_db.find(SU_LUNATICCARROTBEAT)->require.itemid[0]) >= 0) {
		SkillLunaticCarrotBeat2 lunatic2;
		lunatic2.splashSearch(src, target, skill_lv, tick, flag);
	}
	else
		SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}

