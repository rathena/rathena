// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_merchant.hpp"

#include "decoratecart.hpp"
#include "cartrevolution.hpp"
#include "changecart.hpp"
#include "itemappraisal.hpp"
#include "crazyuproar.hpp"
#include "mammonite.hpp"
#include "skill_vending.hpp"

std::unique_ptr<const SkillImpl> SkillFactoryMerchant::create(const e_skill skill_id) const {
	switch (skill_id) {
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
		default:
			return nullptr;
	}
}
