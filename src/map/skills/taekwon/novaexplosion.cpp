// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "novaexplosion.hpp"

#include "map/pc.hpp"
#include "map/status.hpp"

SkillNovaExplosion::SkillNovaExplosion() : SkillImpl(SJ_NOVAEXPLOSING) {
}

void SkillNovaExplosion::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST(BL_PC, src);

	skill_attack(BF_MISC, src, src, target, getSkillId(), skill_lv, tick, flag);

	// We can end Dimension here since the cooldown code is processed before this point.
	if (sc && sc->getSCE(SC_DIMENSION))
		status_change_end(src, SC_DIMENSION);
	else // Dimension not active? Activate the 2 second skill block penalty.
		sc_start(src, sd, SC_NOVAEXPLOSING, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
