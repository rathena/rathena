// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MOB_ML_ENCODER_HPP_
#define _MOB_ML_ENCODER_HPP_

#include "mob.hpp"
#include "map.hpp"
#include "battle.hpp"
#include "status.hpp"
#include <vector>
#include <cstdint>
#include <cmath>

/**
 * ML State Encoder for Monster AI
 * Encodes 64-dimensional state vector from mob_data
 * 
 * State dimensions (as per enhanced_hybrid_ml_monster_ai_architecture_v2.md Section 6.2):
 * - dims 0-11:   Self-Awareness (12 dims)
 * - dims 12-21:  Environmental Awareness (10 dims)
 * - dims 22-29:  Social Awareness (8 dims)
 * - dims 30-41:  Tactical Awareness (12 dims)
 * - dims 42-51:  Temporal Awareness (10 dims)
 * - dims 52-63:  Goal-Oriented Awareness (12 dims)
 */
class MobMLEncoder {
public:
    static constexpr size_t STATE_DIM = 64;
    
    /**
     * Encode monster state to 64-dimensional vector
     * @param md Monster data
     * @return State vector (64 floats)
     */
    static std::vector<float> encode_state(const struct mob_data* md);
    
private:
    // Self-awareness encoding (dims 0-11: 12 dimensions)
    static void encode_self_awareness(const struct mob_data* md, std::vector<float>& state, size_t offset);
    
    // Environmental awareness (dims 12-21: 10 dimensions)
    static void encode_environment(const struct mob_data* md, std::vector<float>& state, size_t offset);
    
    // Social awareness (dims 22-29: 8 dimensions)
    static void encode_social(const struct mob_data* md, std::vector<float>& state, size_t offset);
    
    // Tactical awareness (dims 30-41: 12 dimensions)
    static void encode_tactical(const struct mob_data* md, std::vector<float>& state, size_t offset);
    
    // Temporal awareness (dims 42-51: 10 dimensions)
    static void encode_temporal(const struct mob_data* md, std::vector<float>& state, size_t offset);
    
    // Goal-oriented awareness (dims 52-63: 12 dimensions)
    static void encode_goals(const struct mob_data* md, std::vector<float>& state, size_t offset);
    
    // Helper: Normalize value to [0, 1]
    static inline float normalize(float value, float min, float max) {
        if (max <= min) return 0.0f;
        return (value - min) / (max - min);
    }
    
    // Helper: Clamp value to [0, 1]
    static inline float clamp01(float value) {
        return (value < 0.0f) ? 0.0f : (value > 1.0f) ? 1.0f : value;
    }
    
    // Helper: Count entities of type in range
    static int32 count_entities_in_range(const struct mob_data* md, int32 range, int32 bl_type);
    
    // Helper: Get nearest entity
    static block_list* get_nearest_entity(const struct mob_data* md, int32 range, int32 bl_type);
    
    // Helper: Calculate threat level of entity
    static float calculate_threat_level(const struct mob_data* md, const block_list* target);
};

#endif // _MOB_ML_ENCODER_HPP_
