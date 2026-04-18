// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "crazyweed.hpp"

#include <config/core.hpp>

#include "map/pc.hpp"

// GN_CRAZYWEED
SkillCrazyWeed::SkillCrazyWeed() : SkillImpl(GN_CRAZYWEED) {
}

void SkillCrazyWeed::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 area = skill_get_splash(GN_CRAZYWEED_ATK, skill_lv);
	for( int32 i = 0; i < 3 + (skill_lv/2); i++ ) {
		int32 x1 = x - area + rnd()%(area * 2 + 1);
		int32 y1 = y - area + rnd()%(area * 2 + 1);
		skill_addtimerskill(src,tick+i*150,0,x1,y1,GN_CRAZYWEED_ATK,skill_lv,-1,0);
	}
}


// GN_CRAZYWEED_ATK
SkillCrazyWeedAttack::SkillCrazyWeedAttack() : SkillImpl(GN_CRAZYWEED_ATK) {
}

void SkillCrazyWeedAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	skillratio += -100 + 700 + 100 * skill_lv;
	RE_LVL_DMOD(100);
}
