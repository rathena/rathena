
#include "skill.hpp"
#include "skills.hpp"
#include "swordsman/bash.hpp"
#include "swordsman/provoke.hpp"

int main() {
	Bash bash;
	Provoke provoke;

	bash.castend_damage_id();
	provoke.castend_nodamage_id();

	return 0;
}
