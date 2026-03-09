// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "floralflareroad.hpp"

#include <config/const.hpp>

#include "map/status.hpp"

SkillFloralFlareRoad::SkillFloralFlareRoad() : SkillImpl(AG_FLORAL_FLARE_ROAD) {
}

void SkillFloralFlareRoad::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 50 + 740 * skill_lv + 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillFloralFlareRoad::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}
