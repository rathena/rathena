#ifndef SKILLS_SWORDSMAN_BASH_HPP
#define SKILLS_SWORDSMAN_BASH_HPP

#include "../skill.hpp"
#include "../skills.hpp"


class Bash : public Skill<Bash> {
public:
	int castendDamageId() const {
		return 0;
	};

	Bash() : Skill(e_skill::SM_BASH) {};

private:
	friend class Skill<Bash>;
};

#endif // SKILLS_SWORDSMAN_BASH_HPP
