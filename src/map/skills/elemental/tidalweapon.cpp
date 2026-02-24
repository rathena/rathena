// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tidalweapon.hpp"

#include <common/random.hpp>

#include "map/clif.hpp"
#include "map/elemental.hpp"
#include "map/pc.hpp"
#include "map/status.hpp"

SkillTidalWeapon::SkillTidalWeapon() : SkillImpl(EL_TIDAL_WEAPON) {
}

void SkillTidalWeapon::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += 1400;
}

void SkillTidalWeapon::castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	s_elemental_data *ele = BL_CAST(BL_ELEM, src);
	status_change *tsc = status_get_sc(target);
	sc_type type = skill_get_sc(getSkillId());

	if( src->type == BL_ELEM ) {
		s_elemental_data *ele = BL_CAST(BL_ELEM,src);
		status_change *tsc_ele = status_get_sc(ele);
		sc_type type = SC_TIDAL_WEAPON_OPTION, type2 = SC_TIDAL_WEAPON;

		clif_skill_nodamage(src,*battle_get_master(src),getSkillId(),skill_lv);
		clif_skill_damage( *src, *src, tick, status_get_amotion(src), 0, DMGVAL_IGNORE, 1, getSkillId(), skill_lv, DMG_SINGLE );
		if( (tsc_ele && tsc_ele->getSCE(type2)) || (tsc && tsc->getSCE(type)) ) {
			status_change_end(battle_get_master(src),type);
			status_change_end(src,type2);
		}
		if( rnd()%100 < 50 )
			skill_attack(skill_get_type(getSkillId()),src,src,target,getSkillId(),skill_lv,tick,flag);
		else {
			sc_start(src,src,type2,100,skill_lv,skill_get_time(getSkillId(),skill_lv));
			sc_start(src,battle_get_master(src),type,100,ele->id,skill_get_time(getSkillId(),skill_lv));
		}
		clif_skill_nodamage(src,*src,getSkillId(),skill_lv);
	}
}
