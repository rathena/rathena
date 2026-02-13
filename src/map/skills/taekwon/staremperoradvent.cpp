// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "staremperoradvent.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillStarEmperorAdvent::SkillStarEmperorAdvent() : SkillImplRecursiveDamageSplash(SJ_STAREMPEROR) {
}

void SkillStarEmperorAdvent::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	sc_start(src, target, SC_SILENCE, 50 + 10 * skill_lv, skill_lv, skill_get_time(getSkillId(), skill_lv));
}

void SkillStarEmperorAdvent::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 700 + 200 * skill_lv;
}

void SkillStarEmperorAdvent::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change *sc = status_get_sc(src);
	map_session_data* sd = BL_CAST(BL_PC, src);

	int32 starget = BL_CHAR|BL_SKILL;

	if (sc && sc->getSCE(SC_DIMENSION)) {
		if (sd) {
			// Remove old shields if any exist.
			pc_delspiritball(sd, sd->spiritball, 0);
			sc_start2(src, target, SC_DIMENSION1, 100, skill_lv, status_get_max_sp(src), skill_get_time2(SJ_BOOKOFDIMENSION, 1));
			sc_start2(src, target, SC_DIMENSION2, 100, skill_lv, status_get_max_sp(src), skill_get_time2(SJ_BOOKOFDIMENSION, 1));
		}
		status_change_end(src, SC_DIMENSION);
	}

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), starget,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
