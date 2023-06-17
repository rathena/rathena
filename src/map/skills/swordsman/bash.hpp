#ifndef SKILLS_SWORDSMAN_BASH_HPP
#define SKILLS_SWORDSMAN_BASH_HPP

#include "../skill.hpp"
#include "../skills.hpp"


class Bash : public Skill {
public:
	int castendDamageId() const {
		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
	};

	Bash() : Skill(e_skill::SM_BASH) {};

};

#endif // SKILLS_SWORDSMAN_BASH_HPP
