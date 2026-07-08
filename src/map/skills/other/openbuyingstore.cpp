// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "openbuyingstore.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillOpenBuyingStore::SkillOpenBuyingStore() : SkillImpl(ALL_BUYING_STORE) {
}

void SkillOpenBuyingStore::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd )
	{// players only, skill allows 5 buying slots
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv, buyingstore_setup(sd, MAX_BUYINGSTORE_SLOTS) == 0);
	}
}
