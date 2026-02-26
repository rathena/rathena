// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "glacialstomp.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"

#include "glacialnova.hpp"

SkillGlacialStomp::SkillGlacialStomp() : SkillImplRecursiveDamageSplash(AT_GLACIER_STOMP) {
}

void SkillGlacialStomp::calculateSkillRatio(const Damage*, const block_list* src, const block_list*, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 6400 + 500 * (skill_lv - 1);

	if (const status_change* sc = status_get_sc(src); sc != nullptr && sc->hasSCE(SC_TRUTH_OF_ICE)) {
		const status_data* sstatus = status_get_status_data(*src);

		skillratio += 4 * sstatus->spl;
	}

	// Unlike what the description indicates, the BaseLevel modifier is not part of the condition on SC_TRUTH_OF_ICE
	RE_LVL_DMOD(100);
}

void SkillGlacialStomp::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

	status_change* sc = status_get_sc(src);

	if (sc == nullptr) {
		if (map_session_data* sd = BL_CAST(BL_PC, src); sd != nullptr) {
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		}
		return;
	}

	const status_change_entry *sce = sc->getSCE(SC_GLACIER_SHEILD);

	if (sce == nullptr) {
		if (map_session_data* sd = BL_CAST(BL_PC, src); sd != nullptr) {
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		}
		return;
	}

	if (src->m != sce->val4) {
		if (map_session_data* sd = BL_CAST(BL_PC, src); sd != nullptr) {
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		}
		return;
	}

	// TODO : Should the distance to the player be checked?
	// TODO : the player should be teleported to one cell from the center

	if (!unit_movepos(src, sce->val2, sce->val3, 2, true)) {
		if (map_session_data* sd = BL_CAST(BL_PC, src); sd != nullptr) {
			clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
		}
		return;
	}

	clif_fixpos(*src);

	this->castendDamageId(src, target, skill_lv, tick, flag);
}

void SkillGlacialStomp::splashSearch(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const {
	SkillImplRecursiveDamageSplash::splashSearch(src, target, skill_lv, tick, flag);

	SkillGlacialNova skillnova;
	skillnova.castendPos2(src, 0, 0, 1, tick, flag);
}
