// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "rhythmicalwave.hpp"

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillRhythmicalWave::SkillRhythmicalWave() : SkillImpl(TR_RHYTHMICAL_WAVE) {
}

void SkillRhythmicalWave::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if (flag & 1)
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
	else {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
		map_foreachinallrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR|BL_SKILL, src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
		battle_consume_ammo(sd, getSkillId(), skill_lv); // Consume here since Magic/Misc attacks reset arrow_atk
	}
}

void SkillRhythmicalWave::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 250 + 3650 * skill_lv;
	skillratio += pc_checkskill(sd, TR_STAGE_MANNER) * 25; // !TODO: check Stage Manner ratio
	skillratio += 5 * sstatus->spl;	// !TODO: check SPL ratio

	if (sc != nullptr && sc->hasSCE(SC_MYSTIC_SYMPHONY))
		skillratio += 200 + 1000 * skill_lv;

	RE_LVL_DMOD(100);
}
