// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "slingitem.hpp"

#include "map/clif.hpp"
#include "map/itemdb.hpp"
#include "map/map.hpp"
#include "map/npc.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/script.hpp"

// GN_SLINGITEM
SkillSlingItem::SkillSlingItem() : SkillImpl(GN_SLINGITEM) {
}

void SkillSlingItem::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);
	int32 i = 0;

	if( sd ) {
		i = sd->equip_index[EQI_AMMO];
		if( i < 0 )
			return; // No ammo.
		t_itemid ammo_id = sd->inventory_data[i]->nameid;
		if( ammo_id == 0 )
			return;
		sd->itemid = ammo_id;
		if( itemdb_group.item_exists(IG_BOMB, ammo_id) ) {
			if(battle_check_target(src,target,BCT_ENEMY) > 0) {// Only attack if the target is an enemy.
				if( ammo_id == ITEMID_PINEAPPLE_BOMB )
					map_foreachincell(skill_area_sub,target->m,target->x,target->y,BL_CHAR,src,GN_SLINGITEM_RANGEMELEEATK,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
				else
					skill_attack(BF_WEAPON,src,src,target,GN_SLINGITEM_RANGEMELEEATK,skill_lv,tick,flag);
			} else //Otherwise, it fails, shows animation and removes items.
				clif_skill_fail( *sd, GN_SLINGITEM_RANGEMELEEATK, USESKILL_FAIL );
		} else if (itemdb_group.item_exists(IG_THROWABLE, ammo_id)) {
			switch (ammo_id) {
				case ITEMID_HP_INC_POTS_TO_THROW: // MaxHP +(500 + Thrower BaseLv * 10 / 3) and heals 1% MaxHP
					sc_start2(src, target, SC_PROMOTE_HEALTH_RESERCH, 100, 2, 1, 500000);
					status_percent_heal(target, 1, 0);
					break;
				case ITEMID_HP_INC_POTM_TO_THROW: // MaxHP +(1500 + Thrower BaseLv * 10 / 3) and heals 2% MaxHP
					sc_start2(src, target, SC_PROMOTE_HEALTH_RESERCH, 100, 2, 2, 500000);
					status_percent_heal(target, 2, 0);
					break;
				case ITEMID_HP_INC_POTL_TO_THROW: // MaxHP +(2500 + Thrower BaseLv * 10 / 3) and heals 5% MaxHP
					sc_start2(src, target, SC_PROMOTE_HEALTH_RESERCH, 100, 2, 3, 500000);
					status_percent_heal(target, 5, 0);
					break;
				case ITEMID_SP_INC_POTS_TO_THROW: // MaxSP +(Thrower BaseLv / 10 - 5)% and recovers 2% MaxSP
					sc_start2(src, target, SC_ENERGY_DRINK_RESERCH, 100, 2, 1, 500000);
					status_percent_heal(target, 0, 2);
					break;
				case ITEMID_SP_INC_POTM_TO_THROW: // MaxSP +(Thrower BaseLv / 10)% and recovers 4% MaxSP
					sc_start2(src, target, SC_ENERGY_DRINK_RESERCH, 100, 2, 2, 500000);
					status_percent_heal(target, 0, 4);
					break;
				case ITEMID_SP_INC_POTL_TO_THROW: // MaxSP +(Thrower BaseLv / 10 + 5)% and recovers 8% MaxSP
					sc_start2(src, target, SC_ENERGY_DRINK_RESERCH, 100, 2, 3, 500000);
					status_percent_heal(target, 0, 8);
					break;
				default:
					if (dstsd)
						run_script(sd->inventory_data[i]->script, 0, dstsd->id, fake_nd->id);
					break;
			}
		}
	}
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);// This packet is received twice actually, I think it is to show the animation.
}


// GN_SLINGITEM_RANGEMELEEATK
SkillSlingItemAttack::SkillSlingItemAttack() : WeaponSkillImpl(GN_SLINGITEM_RANGEMELEEATK) {
}

void SkillSlingItemAttack::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_data* sstatus = status_get_status_data(*src);
	status_data* tstatus = status_get_status_data(*target);
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ) {
		switch( sd->itemid ) {	// Starting SCs here instead of do it in skill_additional_effect to simplify the code.
			case ITEMID_COCONUT_BOMB:
				sc_start(src,target, SC_STUN, 5 + sd->status.job_level / 2, skill_lv, 1000 * sd->status.job_level / 3);
				sc_start2(src,target, SC_BLEEDING, 3 + sd->status.job_level / 2, skill_lv, src->id, 1000 * status_get_lv(src) / 4 + sd->status.job_level / 3);
				break;
			case ITEMID_MELON_BOMB:
				sc_start4(src, target, SC_MELON_BOMB, 100, skill_lv, 20 + sd->status.job_level, 10 + sd->status.job_level / 2, 0, 1000 * status_get_lv(src) / 4);
				break;
			case ITEMID_BANANA_BOMB:
				{
					uint16 duration = (battle_config.banana_bomb_duration ? battle_config.banana_bomb_duration : 1000 * sd->status.job_level / 4);

					sc_start(src,target, SC_BANANA_BOMB_SITDOWN, status_get_lv(src) + sd->status.job_level + sstatus->dex / 6 - status_get_lv(target) - tstatus->agi / 4 - tstatus->luk / 5, skill_lv, duration);
					sc_start(src,target, SC_BANANA_BOMB, 100, skill_lv, 30000);
					break;
				}
		}
		sd->itemid = 0;
	}
}

void SkillSlingItemAttack::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if( sd ) {
		switch( sd->itemid ) {
			case ITEMID_APPLE_BOMB:
				skillratio += 200 + status_get_str(src) + status_get_dex(src);
				break;
			case ITEMID_COCONUT_BOMB:
			case ITEMID_PINEAPPLE_BOMB:
				skillratio += 700 + status_get_str(src) + status_get_dex(src);
				break;
			case ITEMID_MELON_BOMB:
				skillratio += 400 + status_get_str(src) + status_get_dex(src);
				break;
			case ITEMID_BANANA_BOMB:
				skillratio += 777 + status_get_str(src) + status_get_dex(src);
				break;
			case ITEMID_BLACK_LUMP:
				skillratio += -100 + (status_get_str(src) + status_get_agi(src) + status_get_dex(src)) / 3;
				break;
			case ITEMID_BLACK_HARD_LUMP:
				skillratio += -100 + (status_get_str(src) + status_get_agi(src) + status_get_dex(src)) / 2;
				break;
			case ITEMID_VERY_HARD_LUMP:
				skillratio += -100 + status_get_str(src) + status_get_agi(src) + status_get_dex(src);
				break;
		}
		RE_LVL_DMOD(100);
	}
}
