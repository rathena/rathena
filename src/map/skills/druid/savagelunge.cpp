// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "savagelunge.hpp"

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/unit.hpp"

#include "skill_factory_druid.hpp"

SkillSavageLunge::SkillSavageLunge() : SkillImplRecursiveDamageSplash(AT_SAVAGE_LUNGE) {
}

void SkillSavageLunge::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (!(flag & 1)) {
		if (!unit_movepos(src, target->x, target->y, 2, true)) {
			map_session_data *sd = BL_CAST(BL_PC, src);
			if (sd) {
				clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
			}
			return;
		}
		clif_blown(src);
	}

	int32 attack_flag = flag | SD_ANIMATION;
	skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, attack_flag);

	if (!(flag & 1)) {
		const int32 madness_stage = get_madness_stage(sc);
		if (madness_stage >= 2) {
			map_foreachinrange(apply_splash_outer_sub, target, 3, BL_CHAR, src, getSkillId(), skill_lv, tick, flag,
						target->x, target->y, 0, target->id);
		}
		try_gain_madness(src);
	}
}

void SkillSavageLunge::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	const bool madness = sc && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3));

	int32 skillratio = (madness ? 9000 : 7500) + (madness ? 2000 : 1500) * (skill_lv - 1);
	skillratio += sstatus->pow * 5; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}
