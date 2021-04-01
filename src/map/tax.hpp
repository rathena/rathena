// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _TAX_HPP_
#define _TAX_HPP_

#include <string>
#include <vector>

#include "../common/cbasetypes.hpp"
#include "../common/database.hpp"

#include "pc.hpp"

enum e_tax_type : uint8 {
	TAX_SELLING = 0,
	TAX_BUYING,
	TAX_MAX,
};

struct s_tax_entry {
	uint64 minimal;
	uint16 tax;
};

struct s_tax {
	std::vector<s_tax_entry> each;
	std::vector<s_tax_entry> total;

	uint16 taxPercentage(const std::vector <s_tax_entry> entry, double price);
	void vendingVAT(map_session_data * sd);
	void buyingstoreVAT(map_session_data * sd);
	void inTotalInfo(map_session_data *sd);
};

class TaxDatabase : public TypesafeYamlDatabase<uint64, s_tax> {
public:
	TaxDatabase() : TypesafeYamlDatabase("TAX_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node& node);

	void setVendingTax(map_session_data *sd);
	void setBuyingstoreTax(map_session_data *sd);
};

extern TaxDatabase tax_db;

void tax_reload_vat(void);
void tax_db_reload(void);
void do_init_tax(void);
void do_final_tax(void);

#endif /* _TAX_HPP_ */
