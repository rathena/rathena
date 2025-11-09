// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "run.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillRun::SkillRun() : SkillImpl(TK_RUN) {
}

void SkillRun::castendNoDamageId(block_list *src, block_list *bl, uint16 skill_lv, t_tick tick, int32 &flag) const {
	sc_type type = skill_get_sc(getSkillId());
	status_change *tsc = status_get_sc(bl);
	status_change_entry *tsce = (tsc && type != SC_NONE) ? tsc->getSCE(type) : nullptr;
	map_session_data *sd = BL_CAST(BL_PC, src);

	if (tsce) {
		clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, status_change_end(bl, type));
		return;
	}

	clif_skill_nodamage(src, *bl, getSkillId(), skill_lv, sc_start4(src, bl, type, 100, skill_lv, unit_getdir(bl), 0, 0, 0));
	if (sd) // If the client receives a skill-use packet inmediately before a walkok packet, it will discard the walk packet! [Skotlex]
		clif_walkok(*sd); // So aegis has to resend the walk ok.
}
