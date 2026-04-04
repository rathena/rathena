// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "kitranslation.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillKiTranslation::SkillKiTranslation() : SkillImpl(MO_KITRANSLATION) {
}

void SkillKiTranslation::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if(dstsd && ((dstsd->class_&MAPID_FIRSTMASK) != MAPID_GUNSLINGER && (dstsd->class_&MAPID_SECONDMASK) != MAPID_REBELLION) && dstsd->spiritball < 5) {
		//Require will define how many spiritballs will be transferred
		struct s_skill_condition require;
		require = skill_get_requirement(sd,getSkillId(),skill_lv);
		pc_delspiritball(sd,require.spiritball,0);
		for (int32 i = 0; i < require.spiritball; i++)
			pc_addspiritball(dstsd,skill_get_time(getSkillId(),skill_lv),5);
	} else {
		if(sd)
			clif_skill_fail( *sd, getSkillId() );
		flag |= SKILL_NOCONSUME_REQ;
	}
}
