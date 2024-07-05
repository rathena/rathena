// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAP_SKILL_HPP
#define MAP_SKILL_HPP

#include <array>
#include <string>

#include "map/skill.hpp"

class SkillNotImplementedException : public std::logic_error {
public:
	explicit SkillNotImplementedException(const std::string &what_arg) : std::logic_error(what_arg) {};
	explicit SkillNotImplementedException(uint16_t skill_id) : std::logic_error("Skill " + std::to_string(skill_id) + " not implemented") {};
};

class Skill {
public:
	uint16_t getSkillId() const;

	virtual ~Skill() = default;

	explicit Skill(e_skill skillid) : skill_id_(static_cast<uint16_t>(skillid)) {};

	int castendDamage(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int flag) const;
	int castendNoDamage(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int flag) const;

protected:
	virtual int castendDamageImpl(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int flag) const;
	virtual int castendNoDamageImpl(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int flag) const;
	virtual int castendPositionImpl() const;
	uint16_t skill_id_;
};

class WeaponSkill : public Skill {
public:
	explicit WeaponSkill(e_skill skill_id) : Skill(skill_id) {};
protected:
    virtual int castendDamageImpl(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int flag) const override;
};

#endif // MAP_SKILL_HPP
