// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "icecharm.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillIceCharm::SkillIceCharm() : SkillImpl(KO_HYOUHU_HUBUKI) {
}

void SkillIceCharm::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		int32 ele_type = skill_get_ele(getSkillId(),skill_lv);
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		pc_addspiritcharm(sd,skill_get_time(getSkillId(),skill_lv),MAX_SPIRITCHARM,ele_type);
	}
}
