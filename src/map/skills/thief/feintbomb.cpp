// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "feintbomb.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillFeintBomb::SkillFeintBomb() : WeaponSkillImpl(SC_FEINTBOMB) {
}

void SkillFeintBomb::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + (skill_lv + 1) * sstatus->dex / 2 * ((sd) ? sd->status.job_level / 10 : 1);
	RE_LVL_DMOD(120);
}

void SkillFeintBomb::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	std::shared_ptr<s_skill_unit_group> group = skill_unitsetting(src,getSkillId(),skill_lv,x,y,0); // Set bomb on current Position

	if( group == nullptr || group->unit == nullptr ) {
		if (sd)
			clif_skill_fail( *sd, getSkillId() );
		flag |= SKILL_NOCONSUME_REQ;
		return;
	}
	map_foreachinallrange(unit_changetarget, src, AREA_SIZE, BL_MOB, src, group->unit); // Release all targets against the caster
	skill_blown(src, src, skill_get_blewcount(getSkillId(), skill_lv), unit_getdir(src), BLOWN_IGNORE_NO_KNOCKBACK); // Don't stop the caster from backsliding if special_state.no_knockback is active
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv, false);
	sc_start(src, src, skill_get_sc(getSkillId()), 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
}
