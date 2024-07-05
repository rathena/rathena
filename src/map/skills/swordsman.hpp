#pragma once

#include "skillrepository.hpp"

class Provoke : public Skill {
public:
    Provoke();

    virtual int castendNoDamageImpl(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int flag) const override;
};

void init_swordsman_skills(SkillRepository& repo);
