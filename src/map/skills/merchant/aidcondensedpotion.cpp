// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "aidcondensedpotion.hpp"

#include "map/clif.hpp"
#include "map/itemdb.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/script.hpp"
#include "map/status.hpp"

SkillAidCondensedPotion::SkillAidCondensedPotion() : SkillImpl(CR_SLIMPITCHER) {
}

void SkillAidCondensedPotion::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	map_session_data* dstsd = BL_CAST( BL_PC, target );
	status_data* tstatus = status_get_status_data(*target);
	status_change *tsc = status_get_sc(target);

	// Updated to block Slim Pitcher from working on barricades and guardian stones.
	if (dstmd && (dstmd->mob_id == MOBID_EMPERIUM || status_get_class_(target) == CLASS_BATTLEFIELD))
		return;
	if (potion_hp || potion_sp) {
		int32 hp = potion_hp, sp = potion_sp;
		hp = hp * (100 + (tstatus->vit * 2))/100;
		sp = sp * (100 + (tstatus->int_ * 2))/100;
		if (dstsd) {
			if (hp)
				hp = hp * (100 + pc_checkskill(dstsd,SM_RECOVERY)*10 + pc_skillheal2_bonus(dstsd, getSkillId()))/100;
			if (sp)
				sp = sp * (100 + pc_checkskill(dstsd,MG_SRECOVERY)*10 + pc_skillheal2_bonus(dstsd, getSkillId()))/100;
		}
		if (tsc != nullptr && !tsc->empty()) {
			uint8 penalty = 0;

			if (tsc->getSCE(SC_WATER_INSIGNIA) && tsc->getSCE(SC_WATER_INSIGNIA)->val1 == 2) {
				hp += hp / 10;
				sp += sp / 10;
			}
			if (tsc->getSCE(SC_CRITICALWOUND))
				penalty += tsc->getSCE(SC_CRITICALWOUND)->val2;
			if (tsc->getSCE(SC_DEATHHURT) && tsc->getSCE(SC_DEATHHURT)->val3 == 1)
				penalty += 20;
			if (tsc->getSCE(SC_NORECOVER_STATE))
				penalty = 100;
			if (penalty > 0) {
				hp -= hp * penalty / 100;
				sp -= sp * penalty / 100;
			}
		}
		if(hp > 0)
			clif_skill_nodamage(nullptr,*target,AL_HEAL,hp);
		if(sp > 0)
			clif_skill_nodamage(nullptr,*target,MG_SRECOVERY,sp);
		status_heal(target,hp,sp,0);
	}
}

void SkillAidCondensedPotion::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if (sd) {
		int32 i_lv = 0, j = 0;
		struct s_skill_condition require = skill_get_requirement(sd, getSkillId(), skill_lv);
		i_lv = skill_lv%11 - 1;
		j = pc_search_inventory(sd, require.itemid[i_lv]);
		if (j < 0 || require.itemid[i_lv] <= 0 || sd->inventory_data[j] == nullptr || sd->inventory.u.items_inventory[j].amount < require.amount[i_lv])
		{
			clif_skill_fail( *sd, getSkillId() );
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}
		potion_flag = 1;
		potion_hp = 0;
		potion_sp = 0;
		run_script(sd->inventory_data[j]->script,0,sd->id,0);
		potion_flag = 0;
		//Apply skill bonuses
		i_lv = pc_checkskill(sd,CR_SLIMPITCHER)*10
			+ pc_checkskill(sd,AM_POTIONPITCHER)*10
			+ pc_checkskill(sd,AM_LEARNINGPOTION)*5
			+ pc_skillheal_bonus(sd, getSkillId());

		potion_hp = potion_hp * (100+i_lv)/100;
		potion_sp = potion_sp * (100+i_lv)/100;

		// Final heal increased by HPlus.
		// Is this the right place for this??? [Rytech]
		// Can HPlus also affect SP recovery???
		status_data* sstatus = status_get_status_data(*src);

		if (sstatus && sstatus->hplus > 0) {
			potion_hp += potion_hp * sstatus->hplus / 100;
			potion_sp += potion_sp * sstatus->hplus / 100;
		}

		if(potion_hp > 0 || potion_sp > 0) {
			i_lv = skill_get_splash(getSkillId(), skill_lv);
			map_foreachinallarea(skill_area_sub,
				src->m,x-i_lv,y-i_lv,x+i_lv,y+i_lv,BL_CHAR,
				src,getSkillId(),skill_lv,tick,flag|BCT_PARTY|BCT_GUILD|1,
				skill_castend_nodamage_id);
		}
	} else {
		struct item_data *item = itemdb_search(skill_db.find(getSkillId())->require.itemid[skill_lv - 1]);
		int32 id = skill_get_max(CR_SLIMPITCHER) * 10;

		potion_flag = 1;
		potion_hp = 0;
		potion_sp = 0;
		run_script(item->script,0,src->id,0);
		potion_flag = 0;
		potion_hp = potion_hp * (100+id)/100;
		potion_sp = potion_sp * (100+id)/100;

		if(potion_hp > 0 || potion_sp > 0) {
			id = skill_get_splash(getSkillId(), skill_lv);
			map_foreachinallarea(skill_area_sub,
				src->m,x-id,y-id,x+id,y+id,BL_CHAR,
				src,getSkillId(),skill_lv,tick,flag|BCT_PARTY|BCT_GUILD|1,
					skill_castend_nodamage_id);
		}
	}
}
