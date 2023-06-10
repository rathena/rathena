// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAP_SKILL_HPP
#define MAP_SKILL_HPP

#include <array>
#include <string>

#include "skills.hpp"

constexpr int MAX_SKILL_LEVEL = 13;


template <typename T>
class Skill {
public:
	int castend_damage_id() const {
		return as_underlying().castendDamageId();
	};
	int castend_nodamage_id() const {
		return as_underlying().castendNoDamageId();
	};
	int castend_pos2() const {
		return as_underlying().castendPos2();
	};

	uint16_t getSkillID() const {
		return nameid;
	}

protected:
	explicit Skill(e_skill skillid) : nameid(static_cast<uint16_t>(skillid)) {};
private:
	friend T;

	uint16_t nameid;
	std::string name;
	std::string desc;
	std::array<int32_t, MAX_SKILL_LEVEL> range;

	inline T& as_underlying() {
		return static_cast<T&>(*this);
	}

	inline const T& as_underlying() const {
		return static_cast<const T&>(*this);
	}
};


#endif // MAP_SKILL_HPP
