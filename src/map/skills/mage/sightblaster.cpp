// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sightblaster.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillSightBlaster::SkillSightBlaster() : SkillImpl(WZ_SIGHTBLASTER) {
}

void SkillSightBlaster::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start2(src,target,skill_get_sc(getSkillId()),100,skill_lv,getSkillId(),skill_get_time(getSkillId(),skill_lv)));
}

void SkillSightBlaster::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	// Calculate and apply damage without display
	const e_skill skill_id = getSkillId();
	int64 damage = skill_attack(BF_MAGIC, src, src, target, skill_id, skill_lv, tick, flag | SD_NODISPLAY);

	// Handle display ourselves - get damage type from skill_db, handle SD_LEVEL flag
	status_data* sstatus = status_get_status_data(*src);
	status_data* tstatus = status_get_status_data(*target);
	clif_skill_damage(*src, *target, tick, sstatus->amotion, tstatus->dmotion, damage, skill_get_num(skill_id, skill_lv), skill_id, (flag & SD_LEVEL) ? -1 : skill_lv, skill_get_hit(skill_id));
}

void SkillSightBlaster::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	base_skillratio += 500;
#endif
}
