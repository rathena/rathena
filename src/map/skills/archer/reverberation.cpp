// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "reverberation.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillReverberation::SkillReverberation() : SkillImpl(WM_REVERBERATION) {
}

void SkillReverberation::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change_end(target, SC_SOUNDBLEND);
}

void SkillReverberation::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_change *tsc = status_get_sc(target);

	// MATK [{(Skill Level x 300) + 400} x Casters Base Level / 100] %
	skillratio += -100 + 700 + 300 * skill_lv;
	RE_LVL_DMOD(100);
	if (tsc && tsc->getSCE(SC_SOUNDBLEND))
		skillratio += skillratio * 50 / 100;
}

void SkillReverberation::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	else {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR|BL_SKILL, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
		battle_consume_ammo(sd, getSkillId(), skill_lv); // Consume here since Magic/Misc attacks reset arrow_atk
	}
}
