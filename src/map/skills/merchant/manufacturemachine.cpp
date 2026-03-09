// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "manufacturemachine.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillManufactureMachine::SkillManufactureMachine() : SkillImpl(MT_M_MACHINE) {
}

void SkillManufactureMachine::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		sd->skill_id_old = getSkillId();
		sd->skill_lv_old = skill_lv;

		clif_cooking_list( *sd, 31, getSkillId(), 1, 7 );
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}
}
