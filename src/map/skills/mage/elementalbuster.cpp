// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "elementalbuster.hpp"

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

// EM_ELEMENTAL_BUSTER
SkillElementalBuster::SkillElementalBuster() : SkillImpl(EM_ELEMENTAL_BUSTER) {
}

void SkillElementalBuster::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd == nullptr)
		return;

	if (!sd->ed || !(sd->ed->elemental.class_ >= ELEMENTALID_DILUVIO && sd->ed->elemental.class_ <= ELEMENTALID_SERPENS)) {
		clif_skill_fail( *sd, getSkillId() );
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	uint16 buster_element;

	switch (sd->ed->elemental.class_) {
		case ELEMENTALID_ARDOR:
			buster_element = EM_ELEMENTAL_BUSTER_FIRE;
			break;
		case ELEMENTALID_DILUVIO:
			buster_element = EM_ELEMENTAL_BUSTER_WATER;
			break;
		case ELEMENTALID_PROCELLA:
			buster_element = EM_ELEMENTAL_BUSTER_WIND;
			break;
		case ELEMENTALID_TERREMOTUS:
			buster_element = EM_ELEMENTAL_BUSTER_GROUND;
			break;
		case ELEMENTALID_SERPENS:
			buster_element = EM_ELEMENTAL_BUSTER_POISON;
			break;
	}

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src, *target, buster_element, skill_lv);// Animation for the triggered blaster element.
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);// Triggered after blaster animation to make correct skill name scream appear.
	map_foreachinrange(skill_area_sub, target, 6, BL_CHAR | BL_SKILL, src, buster_element, skill_lv, tick, flag | BCT_ENEMY | SD_LEVEL | SD_SPLASH | 1, skill_castend_damage_id);
}


// EM_ELEMENTAL_BUSTER_FIRE
SkillElementalBusterFire::SkillElementalBusterFire() : SkillImplRecursiveDamageSplash(EM_ELEMENTAL_BUSTER_FIRE) {
}

void SkillElementalBusterFire::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 550 + 2650 * skill_lv;
	skillratio += 10 * sstatus->spl;
	if (tstatus->race == RC_FORMLESS || tstatus->race == RC_DRAGON)
		skillratio += 150 * skill_lv;
	RE_LVL_DMOD(100);
}


// EM_ELEMENTAL_BUSTER_GROUND
SkillElementalBusterGround::SkillElementalBusterGround() : SkillImplRecursiveDamageSplash(EM_ELEMENTAL_BUSTER_GROUND) {
}

void SkillElementalBusterGround::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 550 + 2650 * skill_lv;
	skillratio += 10 * sstatus->spl;
	if (tstatus->race == RC_FORMLESS || tstatus->race == RC_DRAGON)
		skillratio += 150 * skill_lv;
	RE_LVL_DMOD(100);
}


// EM_ELEMENTAL_BUSTER_POISON
SkillElementalBusterPoison::SkillElementalBusterPoison() : SkillImplRecursiveDamageSplash(EM_ELEMENTAL_BUSTER_POISON) {
}

void SkillElementalBusterPoison::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 550 + 2650 * skill_lv;
	skillratio += 10 * sstatus->spl;
	if (tstatus->race == RC_FORMLESS || tstatus->race == RC_DRAGON)
		skillratio += 150 * skill_lv;
	RE_LVL_DMOD(100);
}


// EM_ELEMENTAL_BUSTER_WATER
SkillElementalBusterWater::SkillElementalBusterWater() : SkillImplRecursiveDamageSplash(EM_ELEMENTAL_BUSTER_WATER) {
}

void SkillElementalBusterWater::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 550 + 2650 * skill_lv;
	skillratio += 10 * sstatus->spl;
	if (tstatus->race == RC_FORMLESS || tstatus->race == RC_DRAGON)
		skillratio += 150 * skill_lv;
	RE_LVL_DMOD(100);
}


// EM_ELEMENTAL_BUSTER_WIND
SkillElementalBusterWind::SkillElementalBusterWind() : SkillImplRecursiveDamageSplash(EM_ELEMENTAL_BUSTER_WIND) {
}

void SkillElementalBusterWind::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_data* tstatus = status_get_status_data(*target);

	skillratio += -100 + 550 + 2650 * skill_lv;
	skillratio += 10 * sstatus->spl;
	if (tstatus->race == RC_FORMLESS || tstatus->race == RC_DRAGON)
		skillratio += 150 * skill_lv;
	RE_LVL_DMOD(100);
}
