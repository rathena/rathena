// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "tax.hpp"

#include <algorithm>
#include <string.h>
#include <yaml-cpp/yaml.h>

#include "../common/cbasetypes.h"
#include "../common/showmsg.h"

#include "buyingstore.hpp"
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
 * Reload tax values for autotrade players (unused)
 */
void tax_reload_vendors(void) {
	// reload VAT price on vendors
}

/**
 * Sets the tax database file name from the map_athena.conf
 */
 void tax_set_conf(const std::string filename) {
	tax_conf = filename;
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

}
