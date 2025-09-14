// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "findstone.hpp"

#include "/mapstatus.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/item.hpp"
#include "map/map.hpp"
#include "map/battle.hpp"

SkillFindStone::SkillFindStone() : SkillImpl(TF_PICKSTONE) {
}

int32 SkillFindStone::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	if (sd) {
		unsigned char eflag;
		struct item item_tmp;
		struct block_list tbl;
		clif_skill_nodamage(src, *bl, this->skill_id, skill_lv);
		memset(&item_tmp, 0, sizeof(item_tmp));
		memset(&tbl, 0, sizeof(tbl)); // [MouseJstr]
		item_tmp.nameid = ITEMID_STONE;
		item_tmp.identify = 1;
		tbl.id = 0;
		// Commented because of duplicate animation [Lemongrass]
		// At the moment this displays the pickup animation a second time
		// If this is required in older clients, we need to add a version check here
		// clif_takeitem(*sd,tbl);
		eflag = pc_additem(sd, &item_tmp, 1, LOG_TYPE_PRODUCE);
		if (eflag) {
			clif_additem(sd, 0, 0, eflag);
			if (battle_config.skill_drop_items_full)
				map_addflooritem(&item_tmp, 1, sd->m, sd->x, sd->y, 0, 0, 0, 4, 0);
		}
	}
	return 1;
}