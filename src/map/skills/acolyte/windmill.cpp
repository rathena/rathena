// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "windmill.hpp"

#include <common/random.hpp>

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWindmill::SkillWindmill() : SkillImplRecursiveDamageSplash(SR_WINDMILL) {
}

void SkillWindmill::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	mob_data* dstmd = BL_CAST(BL_MOB, target);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if( dstsd )
		skill_addtimerskill(src,tick+status_get_amotion(src),target->id,0,0,getSkillId(),skill_lv,BF_WEAPON,0);
	else if( dstmd )
		sc_start(src,target, SC_STUN, 100, skill_lv, 1000 + 1000 * (rnd() %3));
}

void SkillWindmill::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	// ATK [(Caster Base Level + Caster DEX) x Caster Base Level / 100] %
	skillratio += -100 + status_get_lv(src) + sstatus->dex;
	RE_LVL_DMOD(100);
}

void SkillWindmill::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	skill_castend_damage_id(src, src, getSkillId(), skill_lv, tick, flag);
}
