// Simplified BattleSkillFactory Implementation
// This shows the complete pattern for the factory

#include "battle_skill_factory.hpp"
#include <iostream>

// ========== CONSTRUCTOR & SINGLETON ==========
BattleSkillFactory::BattleSkillFactory() {
    // Initialize default skill for unimplemented skills
    default_skill = std::make_shared<DefaultSkill>();
}

BattleSkillFactory& BattleSkillFactory::instance() {
    static BattleSkillFactory factory_instance;
    return factory_instance;
}

// ========== CORE FACTORY METHODS ==========

std::shared_ptr<BattleSkill> BattleSkillFactory::get_skill(uint16 skill_id) const {
    // Thread-safe cache check
    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = skill_cache.find(skill_id);
        if (it != skill_cache.end()) {
            ++cache_hits;
            return it->second;
        }
    }
    
    // Cache miss - create new instance
    ++cache_misses;
    auto skill = create_skill(skill_id);
    
    // Add to cache (thread-safe)
    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        skill_cache[skill_id] = skill;
    }
    
    return skill;
}

std::shared_ptr<BattleSkill> BattleSkillFactory::create_skill(uint16 skill_id) const {
    std::lock_guard<std::mutex> lock(creators_mutex);
    
    auto it = skill_creators.find(skill_id);
    if (it != skill_creators.end()) {
        return it->second(); // Call the creator function
    }
    
    // Return default skill for unimplemented skills
    return default_skill;
}

bool BattleSkillFactory::has_skill(uint16 skill_id) const {
    std::lock_guard<std::mutex> lock(creators_mutex);
    return skill_creators.find(skill_id) != skill_creators.end();
}

void BattleSkillFactory::clear_cache() {
    std::lock_guard<std::mutex> lock(cache_mutex);
    skill_cache.clear();
    cache_hits = 0;
    cache_misses = 0;
}

BattleSkillFactory::CacheStats BattleSkillFactory::get_cache_stats() const {
    std::lock_guard<std::mutex> cache_lock(cache_mutex);
    std::lock_guard<std::mutex> creators_lock(creators_mutex);
    
    return {
        skill_cache.size(),
        skill_creators.size(),
        cache_hits,
        cache_misses
    };
}

// ========== SKILL REGISTRATION ==========

void BattleSkillFactory::register_all_skills() {
    std::cout << "Registering battle skills..." << std::endl;
    
    register_weapon_skills();
    register_magic_skills();
    register_misc_skills();
    register_passive_skills();
    
    auto stats = get_cache_stats();
    std::cout << "Registered " << stats.registered_skills << " battle skills." << std::endl;
}

void BattleSkillFactory::register_weapon_skills() {
    // Example registrations using the template system
    
    // Bash skills (multiple skills use same implementation)
    register_skills<BashSkill>({SM_BASH, MS_BASH});
    
    // Pierce skills
    register_skills<PierceSkill>({KN_PIERCE, ML_PIERCE});
    
    // Individual registrations
    register_skill<MammoniteSkill>(MC_MAMMONITE);
    register_skill<SonicBlowSkill>(AS_SONICBLOW);
    register_skill<BackstabSkill>(RG_BACKSTAP);
    
    // Using the convenience macro
    REGISTER_WEAPON_SKILL(*this, DoubleAttackSkill, AC_DOUBLE, MA_DOUBLE, TF_DOUBLE);
    REGISTER_WEAPON_SKILL(*this, ArrowShowerSkill, AC_SHOWER, MA_SHOWER);
    REGISTER_WEAPON_SKILL(*this, ChargeArrowSkill, AC_CHARGEARROW, MA_CHARGEARROW);
    
    std::cout << "Registered weapon skills." << std::endl;
}

void BattleSkillFactory::register_magic_skills() {
    // Bolt spells
    register_skill<FireboltSkill>(MG_FIREBOLT);
    register_skill<ColdboltSkill>(MG_COLDBOLT);
    register_skill<LightningboltSkill>(MG_LIGHTNINGBOLT);
    
    // Area spells
    register_skill<FireballSkill>(MG_FIREBALL);
    register_skill<StormgustSkill>(WZ_STORMGUST);
    register_skill<SoulStrikeSkill>(MG_SOULSTRIKE);
    
    // Using macro for multiple similar skills
    REGISTER_MAGIC_SKILL(*this, NapalmSkill, MG_NAPALMBEAT, HW_NAPALMVULCAN);
    
    std::cout << "Registered magic skills." << std::endl;
}

void BattleSkillFactory::register_misc_skills() {
    register_skill<ZenyNageSkill>(NJ_ZENYNAGE);
    register_skill<ExtremityFistSkill>(MO_EXTREMITYFIST);
    
    // Trap skills
    REGISTER_MISC_SKILL(*this, TrapSkill, RA_FIRINGTRAP, RA_ICEBOUNDTRAP, RA_CLUSTERBOMB);
    
    std::cout << "Registered misc skills." << std::endl;
}

void BattleSkillFactory::register_passive_skills() {
    // Weapon masteries (all use same base implementation)
    register_skills<WeaponMasterySkill>({
        SM_SWORD, SM_TWOHAND, KN_SPEARMASTERY, AM_AXEMASTERY,
        PR_MACEMASTERY, MO_IRONHAND, AS_KATAR, BA_MUSICALLESSON,
        DC_DANCINGLESSON, SA_ADVANCEDBOOK
    });
    
    std::cout << "Registered passive skills." << std::endl;
}

void BattleSkillFactory::preload_common_skills() {
    // Most frequently used skills that should be cached immediately
    std::vector<uint16> common_skills = {
        SM_BASH, MS_BASH,           // Most common attack skill
        KN_PIERCE, ML_PIERCE,       // Common piercing attacks  
        MG_FIREBOLT, MG_COLDBOLT,   // Common bolt spells
        AC_DOUBLE, MA_DOUBLE,       // Common archer skills
        AS_SONICBLOW,               // Common assassin skill
        0                           // Normal attack (skill_id = 0)
    };
    
    std::cout << "Preloading common skills..." << std::endl;
    for (uint16 skill_id : common_skills) {
        get_skill(skill_id); // This adds to cache
    }
    
    auto stats = get_cache_stats();
    std::cout << "Preloaded " << stats.cached_skills << " common skills." << std::endl;
}

// ========== EXAMPLE USAGE ==========

void example_usage() {
    // Get factory instance
    auto& factory = BattleSkillFactory::instance();
    
    // Register all skills (done once at server startup)
    factory.register_all_skills();
    
    // Preload common skills for performance
    factory.preload_common_skills();
    
    // Use skills in battle calculations
    auto bash_skill = factory.get_skill(SM_BASH);
    auto pierce_skill = factory.get_skill(KN_PIERCE);
    auto unknown_skill = factory.get_skill(99999); // Returns default skill
    
    // Check if skill is registered
    if (factory.has_skill(SM_BASH)) {
        std::cout << "Bash skill is registered!" << std::endl;
    }
    
    // Get cache statistics
    auto stats = factory.get_cache_stats();
    std::cout << "Cache Stats:" << std::endl;
    std::cout << "  Cached skills: " << stats.cached_skills << std::endl;
    std::cout << "  Registered skills: " << stats.registered_skills << std::endl;
    std::cout << "  Cache hits: " << stats.cache_hits << std::endl;
    std::cout << "  Cache misses: " << stats.cache_misses << std::endl;
    
    // Cache hit ratio
    double hit_ratio = (double)stats.cache_hits / (stats.cache_hits + stats.cache_misses);
    std::cout << "  Hit ratio: " << (hit_ratio * 100) << "%" << std::endl;
}

// ========== INTEGRATION EXAMPLE ==========

// How to use in battle.cpp
std::shared_ptr<BattleSkill> get_battle_skill(uint16 skill_id) {
    return BattleSkillFactory::instance().get_skill(skill_id);
}

// Replace switch statement usage:
int32 calculate_skill_ratio_new_way(const Damage* wd, const block_list* src, 
                                   const block_list* target, uint16 skill_id, uint16 skill_lv) {
    auto skill = get_battle_skill(skill_id);
    return skill->calculate_skill_ratio(wd, src, target, skill_lv);
}

// Old way (replace this):
int32 calculate_skill_ratio_old_way(const Damage* wd, const block_list* src,
                                   const block_list* target, uint16 skill_id, uint16 skill_lv) {
    int32 skillratio = 100;
    
    switch(skill_id) {
        case SM_BASH:
        case MS_BASH:
            skillratio += 30 * skill_lv;
            break;
        case KN_PIERCE:
        case ML_PIERCE:
            skillratio += 10 * skill_lv;
            break;
        // ... 500+ more lines of switch cases
        default:
            break;
    }
    
    return skillratio;
} 