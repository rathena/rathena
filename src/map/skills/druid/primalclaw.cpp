// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "primalclaw.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

#include "skill_factory_druid.hpp"

constexpr int32 kClawChainDuration = 5000;

SkillPrimalClaw::SkillPrimalClaw() : SkillImplRecursiveDamageSplash(AT_PRIMAL_CLAW) {
}

void SkillPrimalClaw::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	status_change* sc = status_get_sc(src);

	if (!(flag & 1)) {
		clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
	}

	if (!(flag & 1)) {
		if (!unit_movepos(src, target->x, target->y, 2, true)) {
			map_session_data* sd = BL_CAST(BL_PC, src);
			if (sd) {
				clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
			}
			return;
		}
		clif_blown(src);
	}

	SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

	if (!(flag & 1)) {
		status_change_end(src, SC_FERAL_CLAW);
		sc_start(src, src, SC_PRIMAL_CLAW, 100, skill_lv, kClawChainDuration);

		const int32 madness_stage = get_madness_stage(sc);
		if (madness_stage >= 2) {
			int32 base_radius = skill_get_splash(getSkillId(), skill_lv);
			int32 ring_radius = base_radius + 1;
			if (ring_radius > base_radius) {
				map_foreachinrange(apply_splash_outer_sub, target, ring_radius, BL_CHAR, src, getSkillId(), skill_lv, tick, flag,
					target->x, target->y, base_radius, target->id);
			}
		}

		try_gain_madness(src);
	}
}

void SkillPrimalClaw::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
	const status_change* sc = status_get_sc(src);
	const status_data* sstatus = status_get_status_data(*src);

	const bool madness = sc && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3));

	int32 skillratio = 1100 + 950 * (skill_lv - 1);
	if (madness) {
		skillratio += 800;
	}
	skillratio += sstatus->pow * 5; // TODO - unknown scaling [munkrej]
	RE_LVL_DMOD(100);
	base_skillratio += -100 + skillratio;
}

int64 SkillPrimalClaw::splashDamage(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag | SD_ANIMATION);
}
