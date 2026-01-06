// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_mage.hpp"

#include "../status_skill_impl.hpp"
#include "../weapon_skill_impl.hpp"

#include "coldbolt.hpp"
#include "energycoat.hpp"
#include "fireball.hpp"
#include "firebolt.hpp"
#include "firewall.hpp"
#include "frostdiver.hpp"
#include "napalmbeat.hpp"
#include "lightningbolt.hpp"
#include "sight.hpp"
#include "soulstrike.hpp"
#include "stonecurse.hpp"
#include "thunderstorm.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryMage::create(const e_skill skill_id) const {
	switch (skill_id) {
		case AG_CLIMAX:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case EM_SPELL_ENCHANTING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case HW_MAGICCRASHER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case MG_SIGHT:
			return std::make_unique<SkillSight>();
		case MG_SOULSTRIKE:
			return std::make_unique<SkillSoulStrike>();
		case MG_STONECURSE:
			return std::make_unique<SkillStoneCurse>();
		case MG_LIGHTNINGBOLT:
			return std::make_unique<SkillLightningBolt>();
		case MG_NAPALMBEAT:
			return std::make_unique<SkillNapalmBeat>();
		case MG_FIREWALL:
			return std::make_unique<SkillFireWall>();
		case MG_FIREBOLT:
			return std::make_unique<SkillFireBolt>();
		case MG_FROSTDIVER:
			return std::make_unique<SkillFrostDiver>();
		case MG_ENERGYCOAT:
			return std::make_unique<SkillEnergyCoat>();
		case MG_FIREBALL:
			return std::make_unique<SkillFireBall>();
		case MG_COLDBOLT:
			return std::make_unique<SkillColdBolt>();
		case MG_THUNDERSTORM:
			return std::make_unique<SkillThunderStorm>();
		case PF_DOUBLECASTING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case PF_MEMORIZE:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case SA_REVERSEORCISH:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case WL_MARSHOFABYSS:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case WL_RECOGNIZEDSPELL:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case WL_TELEKINESIS_INTENSE:
			return std::make_unique<StatusSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
