// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "stripshadow.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillStripShadow::SkillStripShadow() : SkillImpl(ABC_STRIP_SHADOW) {
}

void SkillStripShadow::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	bool strip_success = skill_strip_equip(src, target, getSkillId(), skill_lv);

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,strip_success);

	//Nothing stripped.
	if( sd && !strip_success )
		clif_skill_fail( *sd, getSkillId() );
}
