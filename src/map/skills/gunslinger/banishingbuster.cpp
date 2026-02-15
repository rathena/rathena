// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "banishingbuster.hpp"

#include <config/core.hpp>

#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillBanishingBuster::SkillBanishingBuster() : WeaponSkillImpl(RL_BANISHING_BUSTER) {
}

void SkillBanishingBuster::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	skillratio += -100 + 1000 + 200 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillBanishingBuster::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	status_change* tsc = status_get_sc(target);
	map_session_data* sd = BL_CAST(BL_PC, src);
	map_session_data* dstsd = BL_CAST(BL_PC, target);

	if (tsc == nullptr || tsc->empty()) {
		return;
	}

	if (status_isimmune(target)) {
		return;
	}

	if ((dstsd && (dstsd->class_ & MAPID_SECONDMASK) == MAPID_SOUL_LINKER) || rnd() % 100 >= 50 + 5 * skill_lv) {
		if (sd) {
			clif_skill_fail(*sd, getSkillId());
		}
		return;
	}

	uint16 n = skill_lv;

	for (const auto& it : status_db) {
		sc_type status = static_cast<sc_type>(it.first);
		status_change_entry* sce = tsc->getSCE(status);

		if (n <= 0) {
			break;
		}
		if (sce == nullptr) {
			continue;
		}
		if (it.second->flag[SCF_NOBANISHINGBUSTER]) {
			continue;
		}

		switch (status) {
			case SC_WHISTLE: case SC_ASSNCROS: case SC_POEMBRAGI:
			case SC_APPLEIDUN: case SC_HUMMING: case SC_DONTFORGETME:
			case SC_FORTUNE: case SC_SERVICE4U:
				if (!battle_config.dispel_song || sce->val4 == 0) {
					continue;
				}
				break;
			case SC_ASSUMPTIO:
				if (target->type == BL_MOB) {
					continue;
				}
				break;
		}

		if (status == SC_BERSERK || status == SC_SATURDAYNIGHTFEVER) {
			sce->val2 = 0;
		}
		status_change_end(target, status);
		n--;
	}

	if (dstsd) {
		pc_bonus_script_clear(dstsd, BSF_REM_ON_BANISHING_BUSTER);
	}
}
