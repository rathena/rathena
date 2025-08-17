#include "skill_factory_swordsman.hpp"

#include "bash.hpp"

void SkillFactorySwordsman::registerSkills(){
	registerSkill(SM_BASH, std::make_shared<SkillBash>());
}
