// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "plantcultivation.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/mob.hpp"

SkillPlantCultivation::SkillPlantCultivation() : SkillImpl(CR_CULTIVATION) {
}

void SkillPlantCultivation::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd) {
		if( map_count_oncell(src->m,x,y,BL_CHAR,0) > 0 )
		{
			clif_skill_fail( *sd, getSkillId() );
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}
		clif_skill_poseffect( *src, getSkillId(), skill_lv, x, y, tick );
		if (rnd()%100 < 50) {
			clif_skill_fail( *sd, getSkillId() );
		} else {
			TBL_MOB* md = nullptr;
			int32 t, mob_id;

			if (skill_lv == 1)
				mob_id = MOBID_BLACK_MUSHROOM + rnd() % 2;
			else {
				int32 rand_val = rnd() % 100;

				if (rand_val < 30)
					mob_id = MOBID_GREEN_PLANT;
				else if (rand_val < 55)
					mob_id = MOBID_RED_PLANT;
				else if (rand_val < 80)
					mob_id = MOBID_YELLOW_PLANT;
				else if (rand_val < 90)
					mob_id = MOBID_WHITE_PLANT;
				else if (rand_val < 98)
					mob_id = MOBID_BLUE_PLANT;
				else
					mob_id = MOBID_SHINING_PLANT;
			}

			md = mob_once_spawn_sub(src, src->m, x, y, "--ja--", mob_id, "", SZ_SMALL, AI_NONE);
			if (!md)
				return;
			if ((t = skill_get_time(getSkillId(), skill_lv)) > 0)
			{
				if( md->deletetimer != INVALID_TIMER )
					delete_timer(md->deletetimer, mob_timer_delete);
				md->deletetimer = add_timer (tick + t, mob_timer_delete, md->id, 0);
			}
			mob_spawn(md);
		}
	}
}
