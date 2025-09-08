#pragma once

#include "../skill_impl.hpp"
#include "../../battle.hpp"

class SkillMG_STONECURSE : public SkillImpl {
public:
    SkillMG_STONECURSE();
    
    void castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32 flag) const override;
};