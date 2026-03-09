// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "exorcismofmalicioussoul.hpp"

#include <config/const.hpp>

#include "map/pc.hpp"
#include "map/status.hpp"

SkillExorcismOfMaliciousSoul::SkillExorcismOfMaliciousSoul() : SkillImplRecursiveDamageSplash(SOA_EXORCISM_OF_MALICIOUS_SOUL) {
}

void SkillExorcismOfMaliciousSoul::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change *sc = status_get_sc(src);
	const status_change *tsc = status_get_sc(target);
	const map_session_data* sd = BL_CAST( BL_PC, src );

	skillratio += -100 + 150 * skill_lv;
	skillratio += pc_checkskill(sd, SOA_SOUL_MASTERY) * 2;
	skillratio += 1 * sstatus->spl;

	if ((tsc != nullptr && tsc->getSCE(SC_SOULCURSE) != nullptr) || (sc != nullptr && sc->getSCE(SC_TOTEM_OF_TUTELARY) != nullptr))
		skillratio += 100 * skill_lv;

	if (sd != nullptr)
		skillratio *= sd->soulball_old;
	RE_LVL_DMOD(100);
}

void SkillExorcismOfMaliciousSoul::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );

	if( sd != nullptr ){
		// Remove old souls if any exist.
		sd->soulball_old = sd->soulball;
		pc_delsoulball( *sd, sd->soulball, 0 );
	}

	skill_area_temp[1] = 0;
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);
	map_foreachinrange(skill_area_sub, target, skill_get_splash(getSkillId(), skill_lv), BL_CHAR|BL_SKILL,
		src, getSkillId(), skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
}
