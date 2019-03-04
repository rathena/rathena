// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENSE in the main folder
#ifndef HOMUNCULUS2YML_HPP
#define HOMUNCULUS2YML_HPP

#include <yaml-cpp/yaml.h>

#include "../common/mmo.hpp"
#include "nodemapper.hpp"

namespace rathena {
	class homunclus_mapper : public node_mapper {
	public:
		virtual std::string database_type() const {
			return "HOMUNCULUS_DB";
		}

		virtual int version() const {
			return 1;
		}

		virtual std::vector<std::string> file_paths() const {
			return this->standard_paths_with_mode;
		}

		virtual std::string file_name() const {
			return "homunculus_db";
		}

		virtual bool process_file(const std::string& path, const std::string& name_ext) const {
			return sv_readdb(path.c_str(), name_ext.c_str(), ',', 50, 50, MAX_HOMUNCULUS_CLASS, &read_homunculus_sub, false);
		}

		static bool read_homunculus_sub(char* split[], int columns, int current) {
			YAML::Node node, base, growth, evo;

			node["Class"] = atoi(split[0]);
			node["EvoClass"] = atoi(split[1]);
			node["Name"] = split[2];
			node["FoodId"] = atoi(split[3]);
			node["HungryDelay"] = atoi(split[4]);
			node["Race"] = atoi(split[7]); // script_get_constant_str("RC_", atoi(split[7]));
			node["Element"] = atoi(split[8]); // script_get_constant_str("Ele_", atoi(split[8]));
			node["Aspd"] = atoi(split[9]);

			// Base Stats
			base["Size"] = atoi(split[5]); // script_get_constant_str("Size_", atoi(split[5]));
			base["Hp"] = atoi(split[10]);
			base["Sp"] = atoi(split[11]);
			base["Str"] = atoi(split[12]);
			base["Agi"] = atoi(split[13]);
			base["Vit"] = atoi(split[14]);
			base["Int"] = atoi(split[15]);
			base["Dex"] = atoi(split[16]);
			base["Luk"] = atoi(split[17]);
			node["BaseStats"] = base;

			// Growth Stats
			growth["MinHp"] = atoi(split[18]);
			growth["MaxHp"] = atoi(split[19]);
			growth["MinSp"] = atoi(split[20]);
			growth["MaxSp"] = atoi(split[21]);
			growth["MinStr"] = atoi(split[22]);
			growth["MaxStr"] = atoi(split[23]);
			growth["MinAgi"] = atoi(split[24]);
			growth["MaxAgi"] = atoi(split[25]);
			growth["MinVit"] = atoi(split[26]);
			growth["MaxVit"] = atoi(split[27]);
			growth["MinInt"] = atoi(split[28]);
			growth["MaxInt"] = atoi(split[29]);
			growth["MinDex"] = atoi(split[30]);
			growth["MaxDex"] = atoi(split[31]);
			growth["MinLuk"] = atoi(split[32]);
			growth["MaxLuk"] = atoi(split[33]);
			node["GrowthStats"] = growth;

			// Evolution Stats
			evo["Size"] = atoi(split[6]); // script_get_constant_str("Size_", atoi(split[6]));
			evo["MinHp"] = atoi(split[34]);
			evo["MaxHp"] = atoi(split[35]);
			evo["MinSp"] = atoi(split[36]);
			evo["MaxSp"] = atoi(split[37]);
			evo["MinStr"] = atoi(split[38]);
			evo["MaxStr"] = atoi(split[39]);
			evo["MinAgi"] = atoi(split[40]);
			evo["MaxAgi"] = atoi(split[41]);
			evo["MinVit"] = atoi(split[42]);
			evo["MaxVit"] = atoi(split[43]);
			evo["MinInt"] = atoi(split[44]);
			evo["MaxInt"] = atoi(split[45]);
			evo["MinDex"] = atoi(split[46]);
			evo["MaxDex"] = atoi(split[47]);
			evo["MinLuk"] = atoi(split[48]);
			evo["MaxLuk"] = atoi(split[49]);
			node["EvolutionStats"] = evo;

			body[current] = node;

			return true;
		}
	};
}

#endif // !HOMUNCULUS2YML_HPP
