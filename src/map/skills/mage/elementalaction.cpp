// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "elementalaction.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/pc.hpp"

SkillElementalAction::SkillElementalAction() : SkillImpl(SO_EL_ACTION) {
}

void SkillElementalAction::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ) {
		int32 duration = 3000;
		if( !sd->ed )
			return;
		switch(sd->ed->db->class_) {
			case ELEMENTALID_AGNI_M: case ELEMENTALID_AQUA_M:
			case ELEMENTALID_VENTUS_M: case ELEMENTALID_TERA_M:
				duration = 6000;
				break;
			case ELEMENTALID_AGNI_L: case ELEMENTALID_AQUA_L:
			case ELEMENTALID_VENTUS_L: case ELEMENTALID_TERA_L:
				duration = 9000;
				break;
		}
		sd->skill_id_old = getSkillId();
		elemental_action(sd->ed, target, tick);
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
		skill_blockpc_start(*sd, getSkillId(), duration);
	}
}
