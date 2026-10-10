// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "highmagnumbreak.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillHighMagnumBreak::SkillHighMagnumBreak() : SkillImplRecursiveDamageSplash(HN_HIGH_MAGNUM_BREAK) {
}

void SkillHighMagnumBreak::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + 1600 * skill_lv + 15 * pc_checkskill(sd, HN_SELFSTUDY_TATICS);
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillHighMagnumBreak::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
	skill_area_temp[0] = 0;
	skill_area_temp[1] = src->id;
	skill_area_temp[2] = 0;
	this->splashSearch(src, src, skill_lv, tick, flag);
}
