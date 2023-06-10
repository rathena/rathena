
#include "skill.hpp"
#include "skills.hpp"
// #include "swordsman/bash.hpp"
// #include "swordsman/provoke.hpp"

#include "skilllist.hpp"



int main() {

	constexpr int skill_id = SM_BASH;

	const auto &sk = skill_db.at(static_cast<e_skill>(skill_id));

	const SkillImpl sk2 = Bash{};

	std::visit([](auto &skill) {
		skill.getSkillID();

		skill.castend_damage_id(); // error
		// skill.hpp:19:26: error: ‘const class Provoke’ has no member named ‘castendDamageId’; did you mean ‘castendNoDamageId’?
		//    19 |   return as_underlying().castendDamageId();
		//       |          ~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~
		//       |          castendNoDamageId

	}, sk2);


	
	for (auto &it : skill_db) {
		std::visit([](auto &skill) {
			skill.getSkillID();

			// skill.castend_damage_id();
		}, it.second);
	}

	return 0;
}
