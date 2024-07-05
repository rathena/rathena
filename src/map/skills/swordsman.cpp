
#include "swordsman.hpp"

#include "skillrepository.hpp"


#include "map/battle.hpp"
#include "map/clif.hpp"
#include "map/mob.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"
#include "map/unit.hpp"


Provoke::Provoke() : Skill(e_skill::SM_PROVOKE) {};

int Provoke::castendNoDamageImpl(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int flag) const {
    status_data *target_status = status_get_status_data(target);
    if (status_has_mode(target_status, MD_STATUSIMMUNE) || battle_check_undead(target_status->race, target_status->def_ele)) {
        return 1;
    }

    map_session_data *sd = BL_CAST(BL_PC, src);
    mob_data *target_md = BL_CAST(BL_MOB, target);

    sc_type type = skill_get_sc(skill_id_);
    // official chance is 70% + 3% per skill level + srcBaseLevel% - targetBaseLevel%
    int chance = 70 + 3 * skill_lv + status_get_lv(src) - status_get_lv(target);
    int i = sc_start(src, target, type, skill_id_ == SM_SELFPROVOKE ? 100 : chance, skill_lv, skill_get_time(skill_id_, skill_lv));
    if (!i) {
        if (sd) {
            clif_skill_fail(*sd, skill_id_);
        }
        return 0;
    }
    clif_skill_nodamage(src, target, skill_id_ == SM_SELFPROVOKE ? SM_PROVOKE : skill_id_, skill_lv, i);
    unit_skillcastcancel(target, 2);
    
    if (target_md) {
        target_md->state.provoke_flag = src->id;
        mob_target(target_md, src, skill_get_range2(src, skill_id_, skill_lv, true));
    }

    // Provoke can cause Coma even though it's a nodamage skill
    if (sd && battle_check_coma(*sd, *target, BF_MISC)) {
        status_change_start(src, target, SC_COMA, 10000, skill_lv, 0, src->id, 0, 0, SCSTART_NONE);
    }
    return 0;
}


void init_swordsman_skills(SkillRepository& repo) {
    repo.addSkill(e_skill::SM_BASH, std::make_unique<WeaponSkill>(e_skill::SM_BASH));
    repo.addSkill(e_skill::SM_PROVOKE, std::make_unique<Provoke>());
}

