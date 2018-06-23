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

static struct s_tax TaxDB[TAX_MAX];
std::string tax_conf = "conf/store_tax.yml";

/**
 * Returns the tax rate of a given amount.
 * @param entry: Tax data.
 * @param price: Value of item.
 * @return Tax rate
 */
unsigned short s_tax::get_tax(const std::vector <struct s_tax_entry> entry, double price) {
	const auto &tax = std::find_if(entry.begin(), entry.end(),
		[&price](const s_tax_entry &e) { return price >= e.minimal; });
	return tax != entry.end() ? tax->tax : 0;
}

/**
 * Returns the tax type based on selling or buying.
 * @param type: Tax type.
 * @return Tax data.
 */
struct s_tax *tax_get(enum e_tax_type type) {
	if (type < TAX_SELLING || type >= TAX_MAX)
		return NULL;
	return &TaxDB[type];
}

/**
 * Calculates the value after tax for Vendors.
 * @param sd: Player data
 */
void tax_vending_vat(struct map_session_data *sd) {
	s_tax *taxdata;

	nullpo_retv(sd);

	if (battle_config.display_tax_info)
		clif_displaymessage(sd->fd, msg_txt(sd, 776)); // [ Tax Information ]

	taxdata = tax_get(TAX_SELLING);

	for (int i = 0; i < ARRAYLENGTH(sd->vending); i++) {
		char msg[CHAT_SIZE_MAX];
		unsigned short tax;

		if (!sd->vending[i].amount)
			continue;

		tax = taxdata->get_tax(taxdata->each, sd->vending[i].value);
		sd->vending[i].value_vat = tax ? (size_t)(sd->vending[i].value - sd->vending[i].value / 10000. * tax) : sd->vending[i].value;

		if (battle_config.display_tax_info) {
			memset(msg, '\0', CHAT_SIZE_MAX);
			sprintf(msg, msg_txt(sd, 777), itemdb_jname(sd->cart.u.items_cart[sd->vending[i].index].nameid), sd->vending[i].value, '-', tax / 100., sd->vending[i].value_vat); // %s : %u %c %.2f%% => %u
			clif_displaymessage(sd->fd, msg);
		}
	}
}

/**
 * Calculates the value after tax for Buyingstores.
 * @param sd: Player data
 */
void tax_buyingstore_vat(struct map_session_data *sd) {
	s_tax *taxdata;
	nullpo_retv(sd);

	if (battle_config.display_tax_info)
		clif_displaymessage(sd->fd, msg_txt(sd, 776)); // [ Tax Information ]

	taxdata = tax_get(TAX_BUYING);

	for (int i = 0; i < ARRAYLENGTH(sd->buyingstore.items); i++) {
		char msg[CHAT_SIZE_MAX];
		unsigned short tax;

		if (!sd->buyingstore.items[i].nameid)
			continue;

		tax = taxdata->get_tax(taxdata->each, sd->buyingstore.items[i].price);
		sd->buyingstore.items[i].price_vat = (size_t)(sd->buyingstore.items[i].price + sd->buyingstore.items[i].price / 10000. * tax);

		if (battle_config.display_tax_info) {
			memset(msg, '\0', CHAT_SIZE_MAX);
			sprintf(msg, msg_txt(sd, 777), itemdb_jname(sd->buyingstore.items[i].nameid), sd->buyingstore.items[i].price, '+', tax / 100., sd->buyingstore.items[i].price_vat); // %s : %u %c %.2f%% => %u
			clif_displaymessage(sd->fd, msg);
		}
	}
}

/**
 * Reads and parses an entry from the tax database.
 * @param node: YAML node containing the entry.
 * @param taxdata: Tax data.
 * @param count: The sequential index of the current entry.
 * @param source: The source YAML file.
 */
static void tax_readdb_sub(const YAML::Node &node, struct s_tax *taxdata, int *count, const std::string &source) {
	if (node["InTotal"].IsDefined()) {
		for (const auto &tax : node["InTotal"]) {
			if (tax["MinimalValue"].IsDefined() && tax["Tax"].IsDefined()) {
				struct s_tax_entry entry;

				entry.minimal = tax["MinimalValue"].as<size_t>();
				entry.tax = tax["Tax"].as<unsigned short>();
				taxdata->total.push_back(entry);
				std::sort(taxdata->total.begin(), taxdata->total.end(),
					[](const s_tax_entry &a, const s_tax_entry &b) -> bool { return a.minimal > b.minimal; });
				(*count)++;
			}
		}
	}
	if (node["EachEntry"].IsDefined()) {
		for (const auto &tax : node["EachEntry"]) {
			if (tax["MinimalValue"].IsDefined() && tax["Tax"].IsDefined()) {
				struct s_tax_entry entry;

				entry.minimal = tax["MinimalValue"].as<size_t>();
				entry.tax = tax["Tax"].as<unsigned short>();
				taxdata->each.push_back(entry);
				std::sort(taxdata->each.begin(), taxdata->each.end(),
					[](const s_tax_entry &a, const s_tax_entry &b) -> bool { return a.minimal > b.minimal; });
				(*count)++;
			}
		}
	}
}

/**
 * Loads taxes from the tax database.
 */
void tax_readdb(void) {
	YAML::Node config;

	try {
		config = YAML::LoadFile(tax_conf);
	}
	catch (...) {
		ShowError("tax_read_db: Cannot read '" CL_WHITE "%s" CL_RESET "'.\n", tax_conf.c_str());
		return;
	}

	int count = 0;
	if (config["Selling"].IsDefined())
		tax_readdb_sub(config["Selling"], &TaxDB[TAX_SELLING], &count, tax_conf);
	if (config["Buying"].IsDefined())
		tax_readdb_sub(config["Buying"], &TaxDB[TAX_BUYING], &count, tax_conf);

	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'\n", count, tax_conf.c_str());
}

/**
 * Reload value after taxes for players with a Vending/Buyingstore shop.
 */
void tax_reload_vat(void) {
	struct map_session_data *sd = NULL;
	struct s_mapiterator *iter = NULL;

	iter = mapit_getallusers();
	for (sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter)) {
		if (sd->state.vending)
			tax_vending_vat(sd);
		else if (sd->state.buyingstore)
			tax_buyingstore_vat(sd);
	}
	mapit_free(iter);
}

/**
 * Sets the tax database file name from the map_athena.conf
 */
 void tax_set_conf(const std::string filename) {
	tax_conf = filename;
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
	tax_readdb();
}

/**
 * Finalizes the tax database
 */
void do_final_tax(void) {
	for (int i = 0; i < TAX_MAX; i++) {
		TaxDB[i].total.clear();
		TaxDB[i].each.clear();
	}
}
