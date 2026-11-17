// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "feelingthesunmoonandstars.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillFeelingtheSunMoonandStars::SkillFeelingtheSunMoonandStars() : SkillImpl(SG_FEEL) {
}

void SkillFeelingtheSunMoonandStars::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	//AuronX reported you CAN memorize the same map as all three. [Skotlex]
	if (sd) {
		if(!sd->feel_map[skill_lv-1].index)
			clif_feel_req(sd->fd,sd, skill_lv);
		else
			clif_feel_info(sd, skill_lv-1, 1);
	}
}
