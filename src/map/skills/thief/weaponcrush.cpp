// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "weaponcrush.hpp"

#include "map/clif.hpp"
#include "map/pc.hpp"

SkillWeaponCrush::SkillWeaponCrush() : WeaponSkillImpl(GC_WEAPONCRUSH) {
}

void SkillWeaponCrush::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	map_session_data* sd = BL_CAST( BL_PC, src );
	std::shared_ptr<s_skill_unit_group> sg;

	bool i;

	if( (i = skill_strip_equip(src, target, getSkillId(), skill_lv)) )
		clif_skill_nodamage(src,*target,getSkillId(),skill_lv,i);

	//Nothing stripped.
	if( sd && !i )
		clif_skill_fail( *sd, getSkillId() );
}

void SkillWeaponCrush::applyAdditionalEffects(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 attack_type, enum damage_lv dmg_lv) const {
	skill_castend_nodamage_id(src,target,getSkillId(),skill_lv,tick,BCT_ENEMY);
}
