// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_npc.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryNpc::create(const e_skill skill_id) const {
	switch( skill_id ){
		case NPC_ARMORBRAKE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_BARRIER:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_BLEEDING:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_BLEEDING2:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_BLINDATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_CHANGEUNDEAD:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_COMBOATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_CRITICALSLASH:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_CRITICALWOUND:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_CURSEATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_DAMAGE_HEAL:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_DARKCROSS:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_DARKNESSATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_DEFENDER:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_FIREATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_GROUNDATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_GUIDEDATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_HALLUCINATIONWALK:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_HELLPOWER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_HELMBRAKE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_HOLYATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_INVINCIBLE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_KEEPING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_KILLING_AURA:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_MAGICMIRROR:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_MAGMA_ERUPTION:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_MAXPAIN:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_MENTALBREAKER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_PETRIFYATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_PIERCINGATT:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_POISON:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_POISONATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_RANDOMATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_RANGEATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_RELIEVE_OFF:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_RELIEVE_ON:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_SHIELDBRAKE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_SILENCEATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_SLEEPATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_STUNATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_TELEKINESISATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_UNDEADATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_WATERATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NPC_WEAPONBRAKER:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case NPC_WINDATTACK:
			return std::make_unique<WeaponSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
