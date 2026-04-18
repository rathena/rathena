// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "earthdrive.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillEarthDrive::SkillEarthDrive() : SkillImplRecursiveDamageSplash(LG_EARTHDRIVE) {
}

void SkillEarthDrive::castendNoDamageId(block_list* src, block_list* bl, uint16 skill_lv, t_tick tick, int32& flag) const {
	int32 dummy = 1;

	clif_skill_damage( *src, *bl,tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	int32 i = skill_get_splash(getSkillId(),skill_lv);
	map_foreachinallarea(skill_cell_overlap, src->m, src->x-i, src->y-i, src->x+i, src->y+i, BL_SKILL, getSkillId(), &dummy, src);
	map_foreachinrange(skill_area_sub, bl,i,BL_CHAR,src,getSkillId(),skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
	clif_skill_nodamage(src, *src, getSkillId(), skill_lv);
}

void SkillEarthDrive::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 380 * skill_lv + sstatus->str + sstatus->vit; // !TODO: What's the STR/VIT bonus?

	if( sc != nullptr && sc->getSCE( SC_SHIELD_POWER ) ){
		skillratio += skill_lv * 37 * pc_checkskill( sd, IG_SHIELD_MASTERY );
	}

	RE_LVL_DMOD(100);
}
