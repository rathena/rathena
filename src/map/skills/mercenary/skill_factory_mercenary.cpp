#include "skill_factory_mercenary.hpp"

#include "mercenary_bash.hpp"

void SkillFactoryMercenary::registerSkills(){
	registerSkill(MS_BASH, std::make_shared<SkillMercenaryBash>());
}
