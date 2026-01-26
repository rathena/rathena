// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "summonelementaldiluvio.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

// EM_SUMMON_ELEMENTAL_DILUVIO
SkillSummonElementalDiluvio::SkillSummonElementalDiluvio() : SkillImpl(EM_SUMMON_ELEMENTAL_DILUVIO) {
}

void SkillSummonElementalDiluvio::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	if (sd == nullptr)
		return;

	if (sd->ed && sd->ed->elemental.class_ == ELEMENTALID_AQUA_L) {
		// Remove the old elemental before summoning the super one.
		elemental_delete(sd->ed);

		if (!elemental_create(sd, ELEMENTALID_DILUVIO, skill_get_time(getSkillId(), skill_lv))) {
			clif_skill_fail( *sd, getSkillId() );
		} else // Elemental summoned. Buff the player with the bonus.
			sc_start(src, target, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
	} else {
		clif_skill_fail( *sd, getSkillId() );
	}
}


// EM_EL_COLD_FORCE
SkillColdForce::SkillColdForce() : SkillImpl(EM_EL_COLD_FORCE) {
}

void SkillColdForce::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	s_elemental_data *ele = BL_CAST(BL_ELEM, src);

	if( ele ) {
		sc_type type2 = (sc_type)(type-1);
		status_change *esc = status_get_sc(ele);

		if( (esc && esc->getSCE(type2)) || (tsc && tsc->getSCE(type)) ) {
			status_change_end(src,type);
			status_change_end(target,type2);
		} else {
			clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
			sc_start(src,src,type2,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
			sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
		}
	}
}


// EM_EL_CRYSTAL_ARMOR
SkillCrystalArmor::SkillCrystalArmor() : SkillImpl(EM_EL_CRYSTAL_ARMOR) {
}

void SkillCrystalArmor::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());
	s_elemental_data *ele = BL_CAST(BL_ELEM, src);

	if( ele ) {
		sc_type type2 = (sc_type)(type-1);
		status_change *esc = status_get_sc(ele);

		if( (esc && esc->getSCE(type2)) || (tsc && tsc->getSCE(type)) ) {
			status_change_end(src,type);
			status_change_end(target,type2);
		} else {
			clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
			sc_start(src,src,type2,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
			sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
		}
	}
}


// EM_EL_AGE_OF_ICE
SkillAgeOfIce::SkillAgeOfIce() : SkillImplRecursiveDamageSplash(EM_EL_AGE_OF_ICE) {
}

void SkillAgeOfIce::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const s_elemental_data* ed = BL_CAST(BL_ELEM, src);

	base_skillratio += -100 + 3700;
	if (ed)
		base_skillratio += base_skillratio * status_get_lv(ed->master) / 100;
}

void SkillAgeOfIce::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);
}
