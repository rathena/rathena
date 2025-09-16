// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lightningbolt.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillLightningBolt::SkillLightningBolt() : SkillImpl(MG_LIGHTNINGBOLT) {
}

void SkillLightningBolt::calculateSkillRatio(Damage *wd, block_list *src, block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	status_change *sc = status_get_sc(src);

	if (sc) {
		if (sc->getSCE(SC_GRACE_BREEZE_OPTION))
			base_skillratio *= 5;

		if (sc->getSCE(SC_SPELLFIST) && mflag & BF_SHORT) {
			base_skillratio += (sc->getSCE(SC_SPELLFIST)->val3 * 100) + (sc->getSCE(SC_SPELLFIST)->val1 * 50 - 50) - 100;
			// val3 = used bolt level, val1 = used spellfist level. [Rytech]
			wd->div_ = 1; // ad mods, to make it work similar to regular hits [Xazax]
			wd->flag = BF_WEAPON | BF_SHORT;
			wd->type = DMG_NORMAL;
		}
	}
}

void SkillLightningBolt::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}
