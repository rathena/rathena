// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "highnessheal.hpp"

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillHighnessHeal::SkillHighnessHeal() : SkillImpl(AB_HIGHNESSHEAL) {
}

void SkillHighnessHeal::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}

void SkillHighnessHeal::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	status_data* sstatus = status_get_status_data(*src);
	status_change *tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST( BL_PC, src );
	map_session_data* dstsd = BL_CAST( BL_PC, target );

	int32 heal = skill_calc_heal(src, target, getSkillId(), skill_lv, true);

	if (status_isimmune(target) || (dstmd && (status_get_class(target) == MOBID_EMPERIUM || status_get_class_(target) == CLASS_BATTLEFIELD)))
		heal = 0;

	if( tsc != nullptr && !tsc->empty() ) {
		if( tsc->getSCE(SC_KAITE) && !status_has_mode(sstatus,MD_STATUSIMMUNE) ) { //Bounce back heal
			if (--tsc->getSCE(SC_KAITE)->val2 <= 0)
				status_change_end(target, SC_KAITE);
			if (src == target)
				heal=0; //When you try to heal yourself under Kaite, the heal is voided.
			else {
				target = src;
				dstsd = sd;
			}
		}
		else if (tsc->getSCE(SC_BERSERK) || tsc->getSCE(SC_SATURDAYNIGHTFEVER))
			heal = 0; //Needed so that it actually displays 0 when healing.
	}
	clif_skill_nodamage(src, *target, getSkillId(), heal);
	if( tsc && tsc->getSCE(SC_AKAITSUKI) && heal )
		heal = ~heal + 1;
	t_exp heal_get_jobexp = status_heal(target,heal,0,0);

	if(sd && dstsd && heal > 0 && sd != dstsd && battle_config.heal_exp > 0){
		heal_get_jobexp = heal_get_jobexp * battle_config.heal_exp / 100;
		if (heal_get_jobexp <= 0)
			heal_get_jobexp = 1;
		pc_gainexp (sd, target, 0, heal_get_jobexp, 0);
	}
}
