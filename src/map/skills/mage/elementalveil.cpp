// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "elementalveil.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillElementalVeil::SkillElementalVeil() : SkillImpl(EM_ELEMENTAL_VEIL) {
}

void SkillElementalVeil::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	if (sd == nullptr)
		return;

	if (sd->ed && sd->ed->elemental.class_ >= ELEMENTALID_DILUVIO && sd->ed->elemental.class_ <= ELEMENTALID_SERPENS)
		sc_start(src, sd->ed, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	else
		clif_skill_fail( *sd, getSkillId() );
}
