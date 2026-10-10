// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "phantomdagger.hpp"

#include <common/random.hpp>
#include "map/skill.hpp"
#include "map/status.hpp"

SkillPhantomDagger::SkillPhantomDagger() : WeaponSkillImpl(ABC_PHANTOM_DAGGER) {
}

void SkillPhantomDagger::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);

	skillratio += -100 + 1150 * skill_lv + 7000;
	skillratio += 5 * sstatus->pow;
	RE_LVL_DMOD(100);
}

void SkillPhantomDagger::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	dmg.div_ = rnd_chance(15 * skill_lv, 100) ? 4 : 3;
}
