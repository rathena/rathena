// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tax.hpp"

#include <algorithm>
#include <string.h>
#include <yaml-cpp/yaml.h>

#include "../common/cbasetypes.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"

#include "battle.hpp"
#include "buyingstore.hpp"
#include "clif.hpp"
#include "pc.hpp"
#include "vending.hpp"

/*
* Tax Database
*/
TaxDatabase tax_db;

const std::string TaxDatabase::getDefaultLocation() {
	return "conf/tax.yml";
}

/**
* Reads and parses an entry from the tax_db.
* @param node: YAML node containing the entry.
* @return count of successfully parsed rows
*/
uint64 TaxDatabase::parseBodyNode(const YAML::Node &node) {
	std::string type_str;

	if (!this->nodeExists(node, "Type")) {
		return 0;
	}

	if (!this->asString(node, "Type", type_str))
		return 0;

	int64 type;

	if (!script_get_constant(type_str.c_str(), &type)) {
		this->invalidWarning(node["Type"], "Invalid tax type '%s'.\n", type_str.c_str());
		return 0;
	}

	std::shared_ptr<s_tax> taxdata = this->find(type);
	bool exists = taxdata != nullptr;

	if (!exists) {
		taxdata = std::make_shared<s_tax>();
	}

	if (this->nodeExists(node, "InTotal")) {
		taxdata->total.clear();

		for (const auto &taxNode : node["InTotal"]) {
			if (this->nodesExist(taxNode, { "MinimalValue", "Tax" }))
				continue;

			s_tax_entry entry = {};

			if (!this->asUInt64(taxNode, "MinimalValue", entry.minimal)) {
				this->invalidWarning(taxNode["MinimalValue"], "Invalid value, defaulting to 0.");
				entry.minimal = 0;
			}

			if (!this->asUInt16(taxNode, "Tax", entry.tax)) {
				this->invalidWarning(taxNode["Tax"], "Invalid value, defaulting to 0.");
				entry.tax = 0;
			}

			taxdata->total.push_back(entry);

			std::sort(taxdata->total.begin(), taxdata->total.end(),
				[](const s_tax_entry &a, const s_tax_entry &b) -> bool { return a.minimal > b.minimal; });
		}
	}

	if (this->nodeExists(node, "EachEntry")) {
		taxdata->each.clear();

		for (const auto &taxNode : node["EachEntry"]) {
			if (this->nodesExist(taxNode, { "MinimalValue", "Tax" }))
				continue;

			s_tax_entry entry = { 0 };

			if (!this->asUInt64(taxNode, "MinimalValue", entry.minimal)) {
				this->invalidWarning(taxNode["MinimalValue"], "Invalid value, defaulting to 0.");
				entry.minimal = 0;
			}

			if (!this->asUInt16(taxNode, "Tax", entry.tax)) {
				this->invalidWarning(taxNode["Tax"], "Invalid value, defaulting to 0.");
				entry.tax = 0;
			}

			taxdata->each.push_back(entry);

			std::sort(taxdata->each.begin(), taxdata->each.end(),
				[](const s_tax_entry &a, const s_tax_entry &b) -> bool { return a.minimal > b.minimal; });
		}
	}

	if (!exists) {
		this->put(type, taxdata);
	}

	return 1;
}

/*
* Set vending tax to a player
* @param sd Player
*/
void TaxDatabase::setVendingTax(map_session_data *sd)
{
	std::shared_ptr<s_tax> taxdata = this->find(TAX_SELLING);

	if (taxdata != nullptr) {
		taxdata->vendingVAT(sd); // Calculate value after taxes
		taxdata->inTotalInfo(sd);
	}
}

/*
* Set buyingstore tax to a player
* @param sd Player
*/
void TaxDatabase::setBuyingstoreTax(map_session_data *sd)
{
	std::shared_ptr<s_tax> taxdata = this->find(TAX_BUYING);

	if (taxdata != nullptr) {
		taxdata->buyingstoreVAT(sd); // Calculate value after taxes
		taxdata->inTotalInfo(sd);
	}
}

/**
 * Returns the tax rate of a given amount.
 * @param entry: Tax data.
 * @param price: Value of item.
 * @return Tax rate
 */
uint16 s_tax::taxPercentage(const std::vector <s_tax_entry> entry, double price) {
	const auto &tax = std::find_if(entry.begin(), entry.end(),
		[&price](const s_tax_entry &e) { return price >= e.minimal; });
	return tax != entry.end() ? tax->tax : 0;
}

/**
 * Calculates the value after tax for Vendors.
 * @param sd: Player data
 */
void s_tax::vendingVAT(map_session_data *sd) {
	nullpo_retv(sd);

	if (battle_config.display_tax_info)
		clif_displaymessage(sd->fd, msg_txt(sd, 776)); // [ Tax Information ]

	for (int i = 0; i < sd->vend_num; i++) {
		if (sd->vending[i].amount == 0)
			continue;

		uint16 tax = this->taxPercentage(this->each, sd->vending[i].value);

		sd->vending[i].value_vat = tax ? (unsigned int)(sd->vending[i].value - sd->vending[i].value / 10000. * tax) : sd->vending[i].value;

		if (battle_config.display_tax_info) {
			char msg[CHAT_SIZE_MAX];

			sprintf(msg, msg_txt(sd, 777), itemdb_ename(sd->cart.u.items_cart[sd->vending[i].index].nameid), sd->vending[i].value, '-', tax / 100., sd->vending[i].value_vat); // %s : %u %c %.2f%% => %u
			clif_displaymessage(sd->fd, msg);
		}
	}
}

/**
 * Calculates the value after tax for Buyingstores.
 * @param sd: Player data
 */
void s_tax::buyingstoreVAT(map_session_data *sd) {
	nullpo_retv(sd);

	if (battle_config.display_tax_info)
		clif_displaymessage(sd->fd, msg_txt(sd, 776)); // [ Tax Information ]

	for (uint8 i = 0; i < sd->buyingstore.slots; i++) {
		if (sd->buyingstore.items[i].nameid == 0)
			continue;

		uint16 tax = this->taxPercentage(this->each, sd->buyingstore.items[i].price);

		sd->buyingstore.items[i].price_vat = tax ? (unsigned int)(sd->buyingstore.items[i].price + sd->buyingstore.items[i].price / 10000. * tax) : sd->buyingstore.items[i].price;

		if (battle_config.display_tax_info) {
			char msg[CHAT_SIZE_MAX];

			sprintf(msg, msg_txt(sd, 777), itemdb_ename(sd->buyingstore.items[i].nameid), sd->buyingstore.items[i].price, '+', tax / 100., sd->buyingstore.items[i].price_vat); // %s : %u %c %.2f%% => %u
			clif_displaymessage(sd->fd, msg);
		}
	}
}

/*
* Show selling/buying tax for total purchase value
* @param sd Vendor or buyer data
*/
void s_tax::inTotalInfo(map_session_data *sd)
{
	if (!battle_config.display_tax_info || this->total.empty())
		return;

	clif_displaymessage(sd->fd, msg_txt(sd, 778)); // [ Total Transaction Tax ]

	for (const auto &tax : this->total) {
		char msg[CHAT_SIZE_MAX];

		sprintf(msg, msg_txt(sd, 779), tax.tax / 100., tax.minimal); // Tax: %.2f%% Minimal Transaction: %u
		clif_displaymessage(sd->fd, msg);
	}
}

/**
 * Reload value after taxes for players with a Vending/Buyingstore shop.
 */
void tax_reload_vat(void) {
	map_session_data *sd;
	s_mapiterator *iter = mapit_getallusers();
	std::shared_ptr<s_tax> vending_tax = tax_db.find(TAX_SELLING);
	std::shared_ptr<s_tax> buyingstore_tax = tax_db.find(TAX_BUYING);

	for (sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter)) {
		if (sd->state.vending && vending_tax != nullptr) {
			vending_tax->vendingVAT(sd);
			vending_tax->inTotalInfo(sd);
		} else if (sd->state.buyingstore && buyingstore_tax != nullptr) {
			buyingstore_tax->buyingstoreVAT(sd);
			buyingstore_tax->inTotalInfo(sd);
		}
	}
	mapit_free(iter);
}

/**
 * Reloads the tax database
 */
void tax_db_reload(void) {
	do_final_tax();
	do_init_tax();
	tax_reload_vat();
}

/**
 * Initializes the tax database
 */
void do_init_tax(void) {
	tax_db.load();
}

/**
 * Finalizes the tax database
 */
void do_final_tax(void) {
	tax_db.clear();
}
