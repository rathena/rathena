// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "summonelementalterremotus.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillSummonElementalTerremotus::SkillSummonElementalTerremotus() : SkillImpl(EM_SUMMON_ELEMENTAL_TERREMOTUS) {
}

void SkillSummonElementalTerremotus::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	if (sd == nullptr)
		return;

	if (sd->ed && sd->ed->elemental.class_ == ELEMENTALID_TERA_L) {
		// Remove the old elemental before summoning the super one.
		elemental_delete(sd->ed);

		if (!elemental_create(sd, ELEMENTALID_TERREMOTUS, skill_get_time(getSkillId(), skill_lv))) {
			clif_skill_fail( *sd, getSkillId() );
		} else // Elemental summoned. Buff the player with the bonus.
			sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	} else {
		clif_skill_fail( *sd, getSkillId() );
	}
}
