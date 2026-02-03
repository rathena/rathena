// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "meteorstormbuster.hpp"

#include <config/const.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMeteorStormBuster::SkillMeteorStormBuster() : SkillImpl(HN_METEOR_STORM_BUSTER) {
}

void SkillMeteorStormBuster::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src,target,SC_STUN,3*skill_lv,skill_lv,skill_get_time2(getSkillId(),skill_lv));
}

void SkillMeteorStormBuster::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	if (mflag & SKILL_ALTDMG_FLAG) {
		// Fall damage
		skillratio += -100 + 300 + 320 * skill_lv;
	} else {
		// Explosion damage
		skillratio += -100 + 450 + 160 * skill_lv;
	}
	skillratio += pc_checkskill(sd, HN_SELFSTUDY_SOCERY) * 5 * skill_lv;
	skillratio += 3 * sstatus->spl;
	RE_LVL_DMOD(100);
	// After RE_LVL_DMOD calculation, HN_SELFSTUDY_SOCERY amplifies the skill ratio of HN_METEOR_STORM_BUSTER (fall damage) by (skill level)%
	if (mflag & SKILL_ALTDMG_FLAG)
		skillratio += skillratio * pc_checkskill(sd, HN_SELFSTUDY_SOCERY) / 100;
	// SC_RULEBREAK increases the skill ratio after HN_SELFSTUDY_SOCERY
	if (sc && sc->getSCE(SC_RULEBREAK))
		skillratio += skillratio * 50 / 100;
}

void SkillMeteorStormBuster::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC,src,src,target,getSkillId(),skill_lv,tick,flag);
}

void SkillMeteorStormBuster::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( map_getcell(src->m, x, y, CELL_CHKLANDPROTECTOR) ) {
		if( sd != nullptr ){
			clif_skill_fail( *sd, getSkillId(), USESKILL_FAIL );
		}

		flag |= SKILL_NOCONSUME_REQ;
		return;
	}

	int32 splash = skill_get_splash(getSkillId(), skill_lv);

	map_foreachinarea(skill_area_sub, src->m, x - splash, y - splash, x + splash, y + splash, BL_CHAR, src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | SKILL_ALTDMG_FLAG | 1, skill_castend_damage_id);
	skill_unitsetting(src, getSkillId(), skill_lv, x, y, skill_get_unit_interval(getSkillId()));

	for (int32 i = 1; i <= (skill_get_time(getSkillId(), skill_lv) / skill_get_time2(getSkillId(), skill_lv)); i++) {
		skill_addtimerskill(src, tick + (t_tick)i*skill_get_time2(getSkillId(), skill_lv), 0, x, y, getSkillId(), skill_lv, 0, flag);
	}
}
