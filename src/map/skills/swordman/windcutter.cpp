// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "windcutter.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillWindCutter::SkillWindCutter() : SkillImplRecursiveDamageSplash(RK_WINDCUTTER) {
}

void SkillWindCutter::modifyDamageData(Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv) const {
	const map_session_data* sd = BL_CAST(BL_PC, &src);

	if (sd != nullptr) {
		if (sd->status.weapon == W_1HSPEAR || sd->status.weapon == W_2HSPEAR)
			dmg.flag |= BF_LONG;

		if (sd->weapontype1 == W_2HSWORD)
			dmg.div_ = 2;
	}
}

void SkillWindCutter::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const map_session_data* sd = BL_CAST(BL_PC, src);

	if (sd) {
		if (sd->weapontype1 == W_2HSWORD)
			skillratio += -100 + 250 * skill_lv;
		else if (sd->weapontype1 == W_1HSPEAR || sd->weapontype1 == W_2HSPEAR)
			skillratio += -100 + 400 * skill_lv;
		else
			skillratio += -100 + 300 * skill_lv;
	} else
		skillratio += -100 + 300 * skill_lv;
	RE_LVL_DMOD(100);
}

void SkillWindCutter::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	skill_castend_damage_id(src, target, getSkillId(), skill_lv, tick, flag);

	if (skill_area_temp[2] == 0) {
		clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
	}
}
