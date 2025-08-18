// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory.hpp"

#include <common/showmsg.hpp>

#include "./mercenary/skill_factory_mercenary.hpp"
#include "./swordsman/skill_factory_swordsman.hpp"

void SkillFactory::registerSkill(const e_skill skill_id, const std::shared_ptr<SkillImpl>& skill) {
	std::shared_ptr<s_skill_db> skill_entry = skill_db.find(skill_id);

	if (skill_entry != nullptr){
		skill_entry->impl = skill;
	}else{
		ShowError("registerSkill: skill ID %hu not found in skill DB", skill_id);
	}
}

void SkillFactory::registerAllSkills(){
	SkillFactoryMercenary::registerSkills();
	SkillFactorySwordsman::registerSkills();
}
