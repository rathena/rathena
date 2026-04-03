// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "npcpsychicwave.hpp"

SkillNpcPsychicWave::SkillNpcPsychicWave() : SkillImpl(NPC_PSYCHIC_WAVE) {
}

void SkillNpcPsychicWave::calculateSkillRatio(const Damage *wd, const block_list *src, const block_list *target, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const {
	base_skillratio += -100 + 500 * skill_lv;
}

void SkillNpcPsychicWave::castendPos2(block_list* src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32& flag) const {
	flag|=1; // Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	skill_unitsetting(src,getSkillId(),skill_lv,x,y,0);
}

void SkillNpcPsychicWave::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
	const status_change* sc = status_get_sc(&src);

	if( sc != nullptr && !sc->empty() ) {
		static const std::vector<sc_type> types = {
			SC_HEATER_OPTION,
			SC_COOLER_OPTION,
			SC_BLAST_OPTION,
			SC_CURSED_SOIL_OPTION,
			SC_FLAMETECHNIC_OPTION,
			SC_COLD_FORCE_OPTION,
			SC_GRACE_BREEZE_OPTION,
			SC_EARTH_CARE_OPTION,
			SC_DEEP_POISONING_OPTION
		};
		for( sc_type type : types ){
			if( sc->hasSCE( type ) ){
				element = sc->getSCE( type )->val3;
				break;
			}
		}
	}
}
