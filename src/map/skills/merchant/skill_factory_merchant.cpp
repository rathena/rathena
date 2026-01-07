// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_merchant.hpp"

#include <config/core.hpp>

#include "../weapon_skill_impl.hpp"

#include "decoratecart.hpp"
#include "cartrevolution.hpp"
#include "changecart.hpp"
#include "itemappraisal.hpp"
#include "crazyuproar.hpp"
#include "mammonite.hpp"
#include "skill_vending.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryMerchant::create(const e_skill skill_id) const {
	switch (skill_id) {
		case AM_ACIDTERROR:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case BO_ACIDIFIED_ZONE_WATER_ATK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case BO_ACIDIFIED_ZONE_GROUND_ATK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case BO_ACIDIFIED_ZONE_WIND_ATK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case BO_ACIDIFIED_ZONE_FIRE_ATK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
#ifdef RENEWAL
		case CR_ACIDDEMONSTRATION:
			return std::make_unique<WeaponSkillImpl>(skill_id);
#endif
		case GN_SLINGITEM_RANGEMELEEATK:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case MC_CARTDECORATE:
			return std::make_unique<SkillDecorateCart>();
		case MC_CARTREVOLUTION:
			return std::make_unique<SkillCartRevolution>();
		case MC_CHANGECART:
			return std::make_unique<SkillChangeCart>();
		case MC_IDENTIFY:
			return std::make_unique<SkillItemAppraisal>();
		case MC_LOUD:
			return std::make_unique<SkillCrazyUproar>();
		case MC_MAMMONITE:
			return std::make_unique<SkillMammonite>();
		case MC_VENDING:
			return std::make_unique<SkillVending>();
		case MT_TRIPLE_LASER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NC_AXEBOOMERANG:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NC_BOOSTKNUCKLE:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NC_MAGMA_ERUPTION:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NC_PILEBUNKER:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case NC_POWERSWING:
			return std::make_unique<WeaponSkillImpl>(skill_id);
		case WS_CARTTERMINATION:
			return std::make_unique<WeaponSkillImpl>(skill_id);

		default:
			return nullptr;
	}
}
