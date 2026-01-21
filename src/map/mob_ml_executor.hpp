// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MOB_ML_EXECUTOR_HPP_
#define _MOB_ML_EXECUTOR_HPP_

#include "mob.hpp"
#include "mob_ml_gateway.hpp"

/**
 * ML Action Executor
 * Translates ML actions to actual monster behaviors using rAthena functions
 * 
 * Actions (from MLAction enum):
 * 0: IDLE - Stop and wait
 * 1: ATTACK - Attack current/new target
 * 2: MOVE_CLOSER - Approach target
 * 3: MOVE_AWAY - Retreat from target
 * 4: MOVE_RANDOM - Random movement
 * 5: SKILL_1 - Use primary skill
 * 6: SKILL_2 - Use secondary skill
 * 7: SKILL_3 - Use tertiary skill
 * 8: CHANGE_TARGET - Switch to different target
 * 9: FLEE - Full retreat to spawn
 * 255: TRADITIONAL_AI - Fallback (should not be executed)
 */
class MobMLExecutor {
public:
    /**
     * Execute ML decision
     * @param md Monster data
     * @param decision ML decision from gateway
     * @return true if action executed successfully
     */
    static bool execute(struct mob_data* md, const MobMLGateway::MLDecision& decision);
    
private:
    /**
     * Individual action executors
     */
    static bool execute_idle(struct mob_data* md);
    static bool execute_attack(struct mob_data* md);
    static bool execute_move_closer(struct mob_data* md);
    static bool execute_move_away(struct mob_data* md);
    static bool execute_move_random(struct mob_data* md);
    static bool execute_skill(struct mob_data* md, int skill_slot);
    static bool execute_change_target(struct mob_data* md);
    static bool execute_flee(struct mob_data* md);
    
    /**
     * Helper functions
     */
    static block_list* find_alternative_target(struct mob_data* md);
    static bool move_towards(struct mob_data* md, int16 x, int16 y, int32 range);
    static bool move_away_from(struct mob_data* md, int16 x, int16 y, int32 distance);
};

#endif // _MOB_ML_EXECUTOR_HPP_
