#ifndef SKILLS_SWORDSMAN_PROVOKE_HPP
#define SKILLS_SWORDSMAN_PROVOKE_HPP

#include "../skill.hpp"


class Provoke : Skill<Provoke> {
public:
	int castend_nodamage_id() {
		return 0;
	};

	Provoke() : Skill(e_skill::SM_PROVOKE) {};

private:
	friend class Skill<Provoke>;
};

#endif // SKILLS_SWORDSMAN_PROVOKE_HPP
