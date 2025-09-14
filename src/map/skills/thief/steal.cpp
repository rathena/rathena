// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "steal.hpp"

#include "map/status.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"

SkillSteal::SkillSteal() : WeaponSkillImpl(TF_STEAL) {
}


int32 SkillSteal::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	if (sd) {
		if (pc_steal_item(sd, bl, skill_lv))
			clif_skill_nodamage(src, *bl, this->skill_id, skill_lv);
		else
			clif_skill_fail(*sd, this->skill_id, USESKILL_FAIL);
	}
	return 1;
}
