// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "aidberserkpotion.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillAidBerserkPotion::SkillAidBerserkPotion() : SkillImpl(AM_BERSERKPITCHER) {
}

void SkillAidBerserkPotion::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);
	status_data* sstatus = status_get_status_data(*src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	status_data* tstatus = status_get_status_data(*target);
	status_change* tsc = status_get_sc(target);

	int32 j,hp = 0,sp = 0;
	if( dstmd && dstmd->mob_id == MOBID_EMPERIUM ) {
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	if( sd ) {
		int32 x,bonus=100;
		struct s_skill_condition require = skill_get_requirement(sd, getSkillId(), skill_lv);
		x = skill_lv%11 - 1;
		j = pc_search_inventory(sd, require.itemid[x]);
		if (j < 0 || require.itemid[x] <= 0) {
			clif_skill_fail( *sd, getSkillId() );
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}
		if (sd->inventory_data[j] == nullptr || sd->inventory.u.items_inventory[j].amount < require.amount[x]) {
			clif_skill_fail( *sd, getSkillId() );
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}
		if( dstsd && dstsd->status.base_level < (uint32)sd->inventory_data[j]->elv ) {
			clif_skill_fail( *sd, getSkillId() );
			flag |= SKILL_NOCONSUME_REQ;
			return;
		}
		potion_flag = 1;
		potion_hp = potion_sp = potion_per_hp = potion_per_sp = 0;
		potion_target = target->id;
		run_script(sd->inventory_data[j]->script,0,sd->id,0);
		potion_flag = potion_target = 0;
		if( sd->sc.getSCE(SC_SPIRIT) && sd->sc.getSCE(SC_SPIRIT)->val2 == SL_ALCHEMIST )
			bonus += sd->status.base_level;
		if( potion_per_hp > 0 || potion_per_sp > 0 ) {
			hp = tstatus->max_hp * potion_per_hp / 100;
			hp = hp * (100 + pc_checkskill(sd,AM_POTIONPITCHER)*10 + pc_checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
			if( dstsd ) {
				sp = dstsd->status.max_sp * potion_per_sp / 100;
				sp = sp * (100 + pc_checkskill(sd,AM_POTIONPITCHER)*10 + pc_checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
			}
		} else {
			if( potion_hp > 0 ) {
				hp = potion_hp * (100 + pc_checkskill(sd,AM_POTIONPITCHER)*10 + pc_checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
				hp = hp * (100 + (tstatus->vit * 2)) / 100;
				if( dstsd )
					hp = hp * (100 + pc_checkskill(dstsd,SM_RECOVERY)*10) / 100;
			}
			if( potion_sp > 0 ) {
				sp = potion_sp * (100 + pc_checkskill(sd,AM_POTIONPITCHER)*10 + pc_checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
				sp = sp * (100 + (tstatus->int_ * 2)) / 100;
				if( dstsd )
					sp = sp * (100 + pc_checkskill(dstsd,MG_SRECOVERY)*10) / 100;
			}
		}

		if ((bonus = pc_get_itemgroup_bonus_group(sd, IG_POTION, sd->itemgrouphealrate))) {
			hp += hp * bonus / 100;
		}

		if( ( bonus = pc_get_itemgroup_bonus_group( sd, IG_POTION, sd->itemgroupsphealrate ) ) ){
			sp += sp * bonus / 100;
		}

		if( (j = pc_skillheal_bonus(sd, getSkillId())) ) {
			hp += hp * j / 100;
			sp += sp * j / 100;
		}
	} else {
		//Maybe replace with potion_hp, but I'm unsure how that works [Playtester]
		switch (skill_lv) {
			case 1: hp = 45; break;
			case 2: hp = 105; break;
			case 3: hp = 175; break;
			default: hp = 325; break;
		}
		hp = (hp + rnd()%(skill_lv*20+1)) * (150 + skill_lv*10) / 100;
		hp = hp * (100 + (tstatus->vit * 2)) / 100;
		if( dstsd )
			hp = hp * (100 + pc_checkskill(dstsd,SM_RECOVERY)*10) / 100;
	}
	if( dstsd && (j = pc_skillheal2_bonus(dstsd, getSkillId())) ) {
		hp += hp * j / 100;
		sp += sp * j / 100;
	}
	// Final heal increased by HPlus.
	// Is this the right place for this??? [Rytech]
	// Can HPlus also affect SP recovery???
	if (sd && sstatus->hplus > 0) {
		hp += hp * sstatus->hplus / 100;
		sp += sp * sstatus->hplus / 100;
	}
	if (tsc != nullptr && !tsc->empty()) {
		uint8 penalty = 0;

		if (tsc->getSCE(SC_WATER_INSIGNIA) && tsc->getSCE(SC_WATER_INSIGNIA)->val1 == 2) {
			hp += hp / 10;
			sp += sp / 10;
		}
		if (tsc->getSCE(SC_CRITICALWOUND))
			penalty += tsc->getSCE(SC_CRITICALWOUND)->val2;
		if (tsc->getSCE(SC_DEATHHURT) && tsc->getSCE(SC_DEATHHURT)->val3)
			penalty += 20;
		if (tsc->getSCE(SC_NORECOVER_STATE))
			penalty = 100;
		if (penalty > 0) {
			hp -= hp * penalty / 100;
			sp -= sp * penalty / 100;
		}
	}

#ifdef RENEWAL
	if (target->type == BL_HOM)
		hp *= 3; // Heal effectiveness is 3x for Homunculus
#endif

	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	if( hp > 0 )
		clif_skill_nodamage(nullptr,*target,AL_HEAL,hp,1);
	if( sp > 0 )
		clif_skill_nodamage(nullptr,*target,MG_SRECOVERY,sp);
	if (tsc) {
#ifdef RENEWAL
		if (tsc->getSCE(SC_EXTREMITYFIST))
			sp = 0;
#endif
		if (tsc->getSCE(SC_NORECOVER_STATE)) {
			hp = 0;
			sp = 0;
		}
	}
	status_heal(target,hp,sp,0);
}
