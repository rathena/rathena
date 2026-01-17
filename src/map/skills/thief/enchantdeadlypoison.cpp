// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "enchantdeadlypoison.hpp"

#include <config/core.hpp>

#include "map/clif.hpp"
#include "map/status.hpp"

SkillEnchantDeadlyPoison::SkillEnchantDeadlyPoison() : SkillImpl(ASC_EDP) {
}

void SkillEnchantDeadlyPoison::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	// EDP also give +25% WATK poison pseudo element to user.
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,
		sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv)));
#ifdef RENEWAL
	sc_start4(src, src, SC_SUB_WEAPONPROPERTY, 100, ELE_POISON, 25, getSkillId(), 0, skill_get_time(getSkillId(), skill_lv));
#else
	sc_start4(src, src, SC_WATK_ELEMENT, 100, ELE_POISON, 25, 0, 0, skill_get_time(getSkillId(), skill_lv));
#endif
}
