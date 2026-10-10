// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "lexexpiatrix.hpp"

#include "map/map.hpp"
#include "map/pc.hpp"
#include "map/skill.hpp"
#include "map/status.hpp"

SkillLexExpiatrix::SkillLexExpiatrix() : SkillImpl(CD_LEX_EXPIATRIX) {
}

void SkillLexExpiatrix::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& skillratio, int32 mflag) const {
	const status_data* sstatus = status_get_status_data(*src);
	const status_change* sc = status_get_sc(src);
	const map_session_data* sd = BL_CAST(BL_PC, src);

	skillratio += -100 + ((sc != nullptr && sc->hasSCE(SC_COMPETENTIA)) ? 1350 : 1000) * skill_lv + 1500 + 50 * pc_checkskill(sd, CD_FIDUS_ANIMUS);
	skillratio += 5 * sstatus->spl;
	RE_LVL_DMOD(100);
}

void SkillLexExpiatrix::modifyElement(const Damage& dmg, const block_list& src, const block_list& target, uint16 skill_lv, int32& element, int32 flag) const {
	const status_change* sc = status_get_sc(&src);

	if (sc == nullptr)
		return;

	if (sc->hasSCE(SC_ANCILLA))
		element = ELE_NEUTRAL;
}

void SkillLexExpiatrix::castendDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	skill_attack(BF_MAGIC, src, src, target, getSkillId(), skill_lv, tick, flag);
}
