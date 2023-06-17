// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAP_SKILL_HPP
#define MAP_SKILL_HPP

#include <array>
#include <string>

#include "skills.hpp"

constexpr int MAX_SKILL_LEVEL = 13;


class Skill {
public:
	virtual int castendDamageId() const;
	virtual int castendNodamageId() const;
	virtual int castendPos2() const;

	uint16_t getSkillID() const {
		return nameid;
	}

protected:
	explicit Skill(e_skill skillid) : nameid(static_cast<uint16_t>(skillid)) {};
private:

	uint16_t nameid;
	std::string name;
	std::string desc;
	std::array<int32_t, MAX_SKILL_LEVEL> range;
};


#endif // MAP_SKILL_HPP
