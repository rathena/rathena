// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "propertyimmune.hpp"

#include "map/clif.hpp"
#include "map/status.hpp"

SkillPropertyImmune::SkillPropertyImmune() : SkillImpl(NPC_IMMUNE_PROPERTY) {
}

void SkillPropertyImmune::castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32& flag) const {
	sc_type type = skill_get_sc(getSkillId());

	switch (skill_lv) {
		case 1: type = SC_IMMUNE_PROPERTY_NOTHING; break;
		case 2: type = SC_IMMUNE_PROPERTY_WATER; break;
		case 3: type = SC_IMMUNE_PROPERTY_GROUND; break;
		case 4: type = SC_IMMUNE_PROPERTY_FIRE; break;
		case 5: type = SC_IMMUNE_PROPERTY_WIND; break;
		case 6: type = SC_IMMUNE_PROPERTY_DARKNESS; break;
		case 7: type = SC_IMMUNE_PROPERTY_SAINT; break;
		case 8: type = SC_IMMUNE_PROPERTY_POISON; break;
		case 9: type = SC_IMMUNE_PROPERTY_TELEKINESIS; break;
		case 10: type = SC_IMMUNE_PROPERTY_UNDEAD; break;
	}
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv,sc_start(src,target,type,100,skill_lv,skill_get_time(getSkillId(),skill_lv)));
}
