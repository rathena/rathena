// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "frostnova.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillFrostNova::SkillFrostNova() : SkillImpl(WZ_FROSTNOVA) {
}

void SkillFrostNova::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	skill_area_temp[1] = 0;
	map_foreachinshootrange(skill_attack_area, src,
		skill_get_splash(getSkillId(), skill_lv), splash_target(src),
		BF_MAGIC, src, src, getSkillId(), skill_lv, tick, flag, BCT_ENEMY);
}

void SkillFrostNova::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio, int32 mflag) const {
#ifdef RENEWAL
	// In renewal the damage formula is identical to MG_FROSTDIVER
	base_skillratio += 10 * skill_lv;
#else
	base_skillratio += -100 + (100 + skill_lv * 10) * 2 / 3;
#endif
}

void SkillFrostNova::applyAdditionalEffects(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	sc_start(src,target,SC_FREEZE,(sd!=nullptr)?skill_lv*5+33:skill_lv*3+35,skill_lv,skill_get_time2(getSkillId(), skill_lv));
}
