// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "removetrap.hpp"

#include "map/clif.hpp"
#include "map/log.hpp"
#include "map/pc.hpp"

SkillRemoveTrap::SkillRemoveTrap() : SkillImpl(HT_REMOVETRAP) {
}

void SkillRemoveTrap::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd == nullptr ){
		return;
	}

	skill_unit* su = BL_CAST(BL_SKILL, target);
	std::shared_ptr<s_skill_unit_group> sg;
	std::shared_ptr<s_skill_db> skill_group;

	// Players can only remove their own traps or traps on Vs maps.
	if( su && (sg = su->group) && (sg->src_id == src->id || map_flag_vs(target->m)) && ( skill_group = skill_db.find(sg->skill_id) ) && skill_group->inf2[INF2_ISTRAP] )
	{
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		if( !(sg->unit_id == UNT_USED_TRAPS || (sg->unit_id == UNT_ANKLESNARE && sg->val2 != 0 )) )
		{ // prevent picking up expired traps
			if( battle_config.skill_removetrap_type )
			{ // get back all items used to deploy the trap
				for( int32 i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++ )
				{
					if( skill_group->require.itemid[i] > 0 )
					{
						int32 flag2;
						struct item item_tmp;
						memset(&item_tmp,0,sizeof(item_tmp));
						item_tmp.nameid = skill_group->require.itemid[i];
						item_tmp.identify = 1;
						item_tmp.amount = skill_group->require.amount[i];
						if( item_tmp.nameid && (flag2=pc_additem(sd,&item_tmp,item_tmp.amount,LOG_TYPE_OTHER)) ){
							clif_additem(sd,0,0,flag2);
							if (battle_config.skill_drop_items_full)
								map_addflooritem(&item_tmp,item_tmp.amount,sd->m,sd->x,sd->y,0,0,0,4,0);
						}
					}
				}
			}
			else
			{ // get back 1 trap
				struct item item_tmp;
				memset(&item_tmp,0,sizeof(item_tmp));
				item_tmp.nameid = su->group->item_id?su->group->item_id:ITEMID_TRAP;
				item_tmp.identify = 1;
				if( item_tmp.nameid && (flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_OTHER)) )
				{
					clif_additem(sd,0,0,flag);
					if (battle_config.skill_drop_items_full)
						map_addflooritem(&item_tmp,1,sd->m,sd->x,sd->y,0,0,0,4,0);
				}
			}
		}
		skill_delunit(su);
	}else
		clif_skill_fail( *sd, getSkillId() );
}
