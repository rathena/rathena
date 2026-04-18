// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "firebolt.hpp"

#include "map/status.hpp"

SkillFireBolt::SkillFireBolt() : SkillImpl(MG_FIREBOLT) {
}

void SkillFireBolt::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	const status_change *sc = status_get_sc(src);

	if (sc) {
		if (sc->getSCE(SC_FLAMETECHNIC_OPTION))
			base_skillratio *= 5;

		if (sc->getSCE(SC_SPELLFIST) && mflag & BF_SHORT) {
			base_skillratio += (sc->getSCE(SC_SPELLFIST)->val3 * 100) + (sc->getSCE(SC_SPELLFIST)->val1 * 50 - 50) - 100;
			// val3 = used bolt level, val1 = used spellfist level. [Rytech]
		}
	}
}

void SkillFireBolt::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}

void SkillFireBolt::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const status_change* sc = status_get_sc(&src);

	if (sc != nullptr) {
		if (sc->hasSCE(SC_SPELLFIST) && (dmg.miscflag & BF_SHORT)) {
			dmg.div_ = 1; // ad mods, to make it work similar to regular hits [Xazax]
			dmg.flag = BF_WEAPON | BF_SHORT;
			dmg.type = DMG_NORMAL;
		}
	}
}
