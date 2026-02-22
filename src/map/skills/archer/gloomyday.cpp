// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "gloomyday.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillGloomyDay::SkillGloomyDay() : SkillImpl(WM_GLOOMYDAY) {
}

void SkillGloomyDay::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if( dstsd && ( pc_checkskill(dstsd,KN_BRANDISHSPEAR) || pc_checkskill(dstsd,LK_SPIRALPIERCE) ||
			pc_checkskill(dstsd,CR_SHIELDCHARGE) || pc_checkskill(dstsd,CR_SHIELDBOOMERANG) ||
			pc_checkskill(dstsd,PA_SHIELDCHAIN) || pc_checkskill(dstsd,LG_SHIELDPRESS) ) )
		{ // !TODO: Which skills aren't boosted anymore?
			sc_start(src,target,SC_GLOOMYDAY_SK,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
			return;
		}
	sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
}
