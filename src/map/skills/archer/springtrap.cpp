// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "springtrap.hpp"

#include "map/clif.hpp"

SkillSpringTrap::SkillSpringTrap() : SkillImpl(HT_SPRINGTRAP) {
}

void SkillSpringTrap::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	clif_skill_nodamage(src,*target,getSkillId(),skill_lv);

	skill_unit *su=nullptr;
	if((target->type==BL_SKILL) && (su=(skill_unit *)target) && (su->group) ){
		switch(su->group->unit_id){
			case UNT_ANKLESNARE:	// ankle snare
				if (su->group->val2 != 0)
					// if it is already trapping something don't spring it,
					// remove trap should be used instead
					break;
				[[fallthrough]];
			case UNT_BLASTMINE:
			case UNT_SKIDTRAP:
			case UNT_LANDMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_FREEZINGTRAP:
			case UNT_CLAYMORETRAP:
			case UNT_TALKIEBOX:
				su->group->unit_id = UNT_USED_TRAPS;
				clif_changetraplook(target, UNT_USED_TRAPS);
				su->group->limit=DIFF_TICK(tick+1500,su->group->tick);
				su->limit=DIFF_TICK(tick+1500,su->group->tick);
		}
	}
}
