// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "turnundead.hpp"

#include "map/status.hpp"

SkillTurnUndead::SkillTurnUndead() : SkillImpl(PR_TURNUNDEAD) {
}

void SkillTurnUndead::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_data* tstatus = status_get_status_data(*target);

	if (!battle_check_undead(tstatus->race, tstatus->def_ele))
		return;
	skill_attack(BF_MAGIC,src,src,target,getSkillId(), skill_lv, tick, flag);
}
