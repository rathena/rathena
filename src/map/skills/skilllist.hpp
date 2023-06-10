#include <variant>

#include "skill.hpp"
#include "skills.hpp"

#include "swordsman/bash.hpp"
#include "swordsman/provoke.hpp"


using SkillImpl = std::variant<Bash, Provoke>;

std::unordered_map<e_skill, SkillImpl> skill_db = {
	{ e_skill::SM_BASH, Bash{} },
	{ e_skill::SM_PROVOKE, Provoke{} }
};
