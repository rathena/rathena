#ifndef SKILLS_SWORDSMAN_PROVOKE_HPP
#define SKILLS_SWORDSMAN_PROVOKE_HPP

#include "../skill.hpp"


class Provoke : public Skill {
public:
	int castendNoDamageId() const {
		return 0;
	};

	Provoke() : Skill(e_skill::SM_PROVOKE) {};

private:
};

#endif // SKILLS_SWORDSMAN_PROVOKE_HPP
