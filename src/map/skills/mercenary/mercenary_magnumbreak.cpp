// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mercenary_magnumbreak.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/map.hpp"
#include "map/path.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillMercenaryMagnumBreak::SkillMercenaryMagnumBreak() : SkillImpl(MS_MAGNUM) {
}

void SkillMercenaryMagnumBreak::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	if(wd->miscflag == 1)
		base_skillratio += 20 * skill_lv; //Inner 3x3 circle takes 100%+20%*level damage [Playtester]
	else
		base_skillratio += 10 * skill_lv; //Outer 5x5 circle takes 100%+10%*level damage [Playtester]
}

void SkillMercenaryMagnumBreak::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST(BL_PC, src);

	if( flag&1 ) {
		// For players, damage depends on distance, so add it to flag if it is > 1
		// Cannot hit hidden targets
		skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag|SD_ANIMATION|(sd?distance_bl(src, target):0));
	}
}

void SkillMercenaryMagnumBreak::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_area_temp[1] = 0;
	map_foreachinshootrange(skill_area_sub, src, skill_get_splash(getSkillId(), skill_lv), BL_SKILL|BL_CHAR,
		src,getSkillId(),skill_lv,tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
	clif_skill_nodamage(src, *src,getSkillId(),skill_lv);
	// Initiate 20% of your damage becomes fire element.
#ifdef RENEWAL
	sc_start4(src,src,SC_SUB_WEAPONPROPERTY,100,ELE_FIRE,20,getSkillId(),0,skill_get_time2(getSkillId(), skill_lv));
#else
	sc_start4(src,src,SC_WATK_ELEMENT,100,ELE_FIRE,20,0,0,skill_get_time2(getSkillId(), skill_lv));
#endif
}

void SkillMercenaryMagnumBreak::modifyHitRate(int16& hit_rate, const block_list* src, const block_list* target, uint16 skill_lv) const {
	hit_rate += hit_rate * 10 * skill_lv / 100;
}
