// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "starcannon.hpp"

#include <config/core.hpp>

#include <common/ers.hpp>

#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

SkillStarCannon::SkillStarCannon() : SkillImpl(SKE_STAR_CANNON) {
}

void SkillStarCannon::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillStarCannon::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	unit_data* ud = unit_bl2ud( src );

	if( ud == nullptr ){
		return;
	}

	for( const std::shared_ptr<s_skill_unit_group>& sug : ud->skillunits ){
		if( sug->skill_id != SKE_TWINKLING_GALAXY ){
			continue;
		}

		skill_unit* su = sug->unit;

		if( distance_xy( x, y, su->x, su->y ) > skill_get_unit_range( sug->skill_id, sug->skill_lv ) ){
			continue;
		}

		std::shared_ptr<s_skill_unit_group> sg = su->group;

		for( int32 i = 0; i< MAX_SKILLTIMERSKILL; i++ ){
			if( ud->skilltimerskill[i] == nullptr ){
				continue;
			}

			if( ud->skilltimerskill[i]->skill_id != SKE_TWINKLING_GALAXY ){
				continue;
			}

			delete_timer(ud->skilltimerskill[i]->timer, skill_timerskill);
			ers_free(skill_timer_ers, ud->skilltimerskill[i]);
			ud->skilltimerskill[i] = nullptr;
		}

		skill_delunitgroup(sg);

		for (int32 i = 0; i < skill_get_time(getSkillId(), skill_lv) / skill_get_unit_interval(getSkillId()); i++)
			skill_addtimerskill(src, tick + (t_tick)i*skill_get_unit_interval(getSkillId()), 0, x, y, getSkillId(), skill_lv, 0, flag);
		flag |= 1;
		skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
	}
}

void SkillStarCannon::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 250 + 550 * skill_lv;
	skillratio += pc_checkskill(sd, SKE_SKY_MASTERY) * 5 * skill_lv;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}
