// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "yamlupgrade.hpp"

using namespace rathena::tool_yamlupgrade;

static bool upgrade_achievement_db(std::string file, const uint32 source_version);
static bool upgrade_item_db(std::string file, const uint32 source_version);
static bool upgrade_job_stats(std::string file, const uint32 source_version);
static bool upgrade_status_db(std::string file, const uint32 source_version);
static bool upgrade_map_drops_db(std::string file, const uint32 source_version);
static bool upgrade_enchantgrade_db( std::string file, const uint32 source_version );
static bool upgrade_item_group_db( std::string file, const uint32 source_version );
static bool upgrade_skill_db( std::string file, const uint32 source_version );

template<typename Func>
bool process(const std::string &type, uint32 version, const std::vector<std::string> &paths, const std::string &name, Func lambda) {
	for (const std::string &path : paths) {
		const std::string name_ext = name + ".yml";
		const std::string from = path + "/" + name_ext;
		const std::string to = path + "/" + name + "-upgrade.yml";

		inNode.reset();

		if (fileExists(from)) {
			inNode = YAML::LoadFile(from);
			uint32 source_version = getHeaderVersion(inNode);

			if (source_version >= version) {
				continue;
			}

			if (!askConfirmation("Found the file \"%s\", which requires an upgrade.\nDo you want to convert it now? (Y/N)\n", from.c_str())) {
				continue;
			}

			if (fileExists(to)) {
				if (!askConfirmation("The file \"%s\" already exists.\nDo you want to replace it? (Y/N)\n", to.c_str())) {
					continue;
				}
			}

			ShowNotice("Upgrade process has begun.\n");

			std::ofstream outFile;

			body.~Emitter();
			new (&body) YAML::Emitter();
			outFile.open(to);

			if (!outFile.is_open()) {
				ShowError("Can not open file \"%s\" for writing.\n", to.c_str());
				return false;
			}

			prepareHeader(outFile, type, version, name);
			if (inNode["Body"].IsDefined()) {
				prepareBody();

				if (!lambda(path, name_ext, source_version)) {
					outFile.close();
					return false;
				}

				finalizeBody();
				outFile << body.c_str();
			}
			prepareFooter(outFile);
			// Make sure there is an empty line at the end of the file for git
			outFile << "\n";
			outFile.close();
		}
	}

	return true;
}

bool YamlUpgradeTool::initialize( int32 argc, char* argv[] ){
	const std::string path_db = std::string(db_path);
	const std::string path_db_mode = path_db + "/" + DBPATH;
	const std::string path_db_import = path_db + "/" + DBIMPORT;

	// Loads required conversion constants
	if (fileExists(item_db.getDefaultLocation())) {
		item_db.load();
	} else {
		parse_item_constants_txt((path_db_mode + "item_db.txt").c_str());
		parse_item_constants_txt((path_db_import + "item_db.txt").c_str());
	}
	if (fileExists(mob_db.getDefaultLocation())) {
		mob_db.load();
	} else {
		sv_readdb(path_db_mode.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants_txt, false);
		sv_readdb(path_db_import.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants_txt, false);
	}
	if (fileExists(skill_db.getDefaultLocation())) {
		skill_db.load();
	} else {
		sv_readdb(path_db_mode.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants_txt, false);
		sv_readdb(path_db_import.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants_txt, false);
	}

	// Load constants
	#define export_constant_npc(a) export_constant(a)
	#include <map/script_constants.hpp>

	std::vector<std::string> root_paths = {
		path_db,
		path_db_mode,
		path_db_import
	};

	if (!process("ACHIEVEMENT_DB", 2, root_paths, "achievement_db", [](const std::string &path, const std::string &name_ext, uint32 source_version) -> bool {
		return upgrade_achievement_db(path + name_ext, source_version);
	})) {
		return false;
	}

	if (!process("ITEM_DB", 3, root_paths, "item_db", [](const std::string& path, const std::string& name_ext, uint32 source_version) -> bool {
		return upgrade_item_db(path + name_ext, source_version);
		})) {
		return false;
	}

	if (!process("JOB_STATS", 2, root_paths, "job_stats", [](const std::string& path, const std::string& name_ext, uint32 source_version) -> bool {
		return upgrade_job_stats(path + name_ext, source_version);
		})) {
		return false;
	}
	
	if (!process("STATUS_DB", 3, root_paths, "status", [](const std::string& path, const std::string& name_ext, uint32 source_version) -> bool {
		return upgrade_status_db(path + name_ext, source_version);
		})) {
		return false;
	}
	
	if (!process("MAP_DROP_DB", 2, root_paths, "map_drops", [](const std::string& path, const std::string& name_ext, uint32 source_version) -> bool {
		return upgrade_map_drops_db(path + name_ext, source_version);
		})) {
		return 0;
	}

	if( !process( "ENCHANTGRADE_DB", 3, root_paths, "enchantgrade", []( const std::string& path, const std::string& name_ext, uint32 source_version ) -> bool {
		return upgrade_enchantgrade_db( path + name_ext, source_version );
		} ) ){
		return false;
	}
	if( !process( "ITEM_GROUP_DB", 4, root_paths, "item_group_db", []( const std::string& path, const std::string& name_ext, uint32 source_version ) -> bool {
		return upgrade_item_group_db( path + name_ext, source_version );
		} ) ){
		return false;
	}

	if( !process( "SKILL_DB", 4, root_paths, "skill_db", []( const std::string& path, const std::string& name_ext, uint32 source_version ) -> bool {
		return upgrade_skill_db( path + name_ext, source_version );
		} ) ){
		return false;
	}

	return true;
}

// Implementation of the upgrade functions
static bool upgrade_achievement_db(std::string file, const uint32 source_version) {
	size_t entries = 0;

	for (const auto &input : inNode["Body"]) {
		body << YAML::BeginMap;
		body << YAML::Key << "Id" << YAML::Value << input["ID"];

		std::string constant = input["Group"].as<std::string>();

		constant.erase(0, 3); // Remove "AG_"
		if (constant.compare("Chat") == 0) // Chat -> Chatting
			constant.insert(4, "ting");
		else if (constant.compare("Hear") == 0 || constant.compare("See") == 0)
			constant = "Chatting"; // Aegis treats these as general "Talk to NPC" achievements.
		else if (constant.compare("Refine") == 0) { // Refine -> Enchant
			constant.erase(0, 6);
			constant = "Enchant" + constant;
		}
		body << YAML::Key << "Group" << YAML::Value << name2Upper(constant);
		body << YAML::Key << "Name" << YAML::Value << input["Name"];

		if (input["Target"].IsDefined()) {
			body << YAML::Key << "Targets";
			body << YAML::BeginSeq;

			for (const auto &it : input["Target"]) {
				body << YAML::BeginMap;
				body << YAML::Key << "Id" << YAML::Value << it["Id"];
				if (it["MobID"].IsDefined()) {
					uint16 mob_id = it["MobID"].as<uint16>();
					std::string *mob_name = util::umap_find(aegis_mobnames, mob_id);

					if (mob_name == nullptr) {
						ShowWarning("mob_avail reading: Invalid mob-class %hu, Mob not read.\n", mob_id);
						return false;
					}

					body << YAML::Key << "Mob" << YAML::Value << *mob_name;
				}
				if (it["Count"].IsDefined() && it["Count"].as<int32>() > 1)
					body << YAML::Key << "Count" << YAML::Value << it["Count"];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}

		if (input["Condition"].IsDefined())
			body << YAML::Key << "Condition" << YAML::Value << input["Condition"];

		if (input["Map"].IsDefined())
			body << YAML::Key << "Map" << YAML::Value << input["Map"];

		if (input["Dependent"].IsDefined()) {
			body << YAML::Key << "Dependents";
			body << YAML::BeginMap;

			for (const auto &it : input["Dependent"]) {
				body << YAML::Key << it["Id"] << YAML::Value << true;
			}

			body << YAML::EndMap;
		}

		/**
		 * Example usage for adding label at specific version.
		if (source_version < ?) {
			body << YAML::Key << "CustomLabel" << YAML::Value << "Unique";
		}
		*/

		if (input["Reward"].IsDefined()) {
			body << YAML::Key << "Rewards";
			body << YAML::BeginMap;
			if (input["Reward"]["ItemID"].IsDefined()) {
				t_itemid item_id = input["Reward"]["ItemID"].as<t_itemid>();
				std::string *item_name = util::umap_find(aegis_itemnames, item_id);

				if (item_name == nullptr) {
					ShowError("Reward item name for item ID %u is not known.\n", item_id);
					return false;
				}

				body << YAML::Key << "Item" << YAML::Value << *item_name;
			}
			if (input["Reward"]["Amount"].IsDefined() && input["Reward"]["Amount"].as<uint16>() > 1)
				body << YAML::Key << "Amount" << YAML::Value << input["Reward"]["Amount"];
			if (input["Reward"]["Script"].IsDefined())
				body << YAML::Key << "Script" << YAML::Value << input["Reward"]["Script"];
			if (input["Reward"]["TitleID"].IsDefined())
				body << YAML::Key << "TitleId" << YAML::Value << input["Reward"]["TitleID"];
			body << YAML::EndMap;
		}

		body << YAML::Key << "Score" << YAML::Value << input["Score"];

		body << YAML::EndMap;
		entries++;
	}

	ShowStatus("Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' achievements in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}

static bool upgrade_item_db(std::string file, const uint32 source_version) {
	size_t entries = 0;

	for( auto input : inNode["Body"] ){
		// If under version 2
		if( source_version < 2 ){
			// Add armor level to all equipments
			if( input["Type"].IsDefined() && input["Type"].as<std::string>() == "Armor" ){
				input["ArmorLevel"] = 1;
			}
		}

		// TODO: this currently converts all scripts using Literal syntax to normal double quote strings
		body << input;
		entries++;
	}

	ShowStatus("Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' items in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}

static bool upgrade_job_stats(std::string file, const uint32 source_version) {
	size_t entries = 0;

	for (auto input : inNode["Body"]) {
		// If under version 2
		if (source_version < 2) {
			// Field name changes
			if (input["HPFactor"].IsDefined()) {
				input["HpFactor"] = input["HPFactor"].as<uint32>();
				input.remove("HPFactor");
			}
			if (input["HpMultiplicator"].IsDefined()) {
				input["HpIncrease"] = input["HpMultiplicator"].as<uint32>();
				input.remove("HpMultiplicator");
			}
			if (input["HPMultiplicator"].IsDefined()) {
				input["HpIncrease"] = input["HPMultiplicator"].as<uint32>();
				input.remove("HPMultiplicator");
			}
			if (input["SpFactor"].IsDefined()) {
				input["SpIncrease"] = input["SpFactor"].as<uint32>();
				input.remove("SpFactor");
			}
			if (input["SPFactor"].IsDefined()) {
				input["SpIncrease"] = input["SPFactor"].as<uint32>();
				input.remove("SPFactor");
			}
		}

		body << input;
		entries++;
	}

	ShowStatus("Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' job stats in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}

static bool upgrade_status_db(std::string file, const uint32 source_version) {
	size_t entries = 0;

	for (auto input : inNode["Body"]) {
		// If under version 3
		if (source_version < 3) {
			// Rename End to EndOnStart
			if (input["End"].IsDefined()) {
				input["EndOnStart"] = input["End"];
				input.remove("End");
			}
		}

		body << input;
		entries++;
	}

	ShowStatus("Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' statuses in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}

static bool upgrade_map_drops_db(std::string file, const uint32 source_version) {
	size_t entries = 0;

	for( auto input : inNode["Body"] ){
		// If under version 2, adjust the rates from n/10000 to n/100000
		if( source_version < 2 ){
			if (input["GlobalDrops"].IsDefined()) {
				for( auto GlobalDrops : input["GlobalDrops"] ){
					if (GlobalDrops["Rate"].IsDefined()) {
						uint32 val = GlobalDrops["Rate"].as<uint32>() * 10;
						GlobalDrops["Rate"] = val;
					}
				}
			}
			if (input["SpecificDrops"].IsDefined()) {
				for( auto SpecificDrops : input["SpecificDrops"] ){
					if (SpecificDrops["Drops"].IsDefined()) {
						for( auto Drops : SpecificDrops["Drops"] ){
							if (Drops["Rate"].IsDefined()) {
								uint32 val = Drops["Rate"].as<uint32>() * 10;
								Drops["Rate"] = val;
							}
						}
					}
				}
			}
		}

		body << input;
		entries++;
	}

	ShowStatus("Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' rates in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}

static bool upgrade_enchantgrade_db( std::string file, const uint32 source_version ){
	size_t entries = 0;

	for( auto input : inNode["Body"] ){
		// If under version 3
		if( source_version < 3 ){
			if( input["Levels"].IsDefined() ){
				for( auto levelNode : input["Levels"] ){
					if( levelNode["Grades"].IsDefined() ){
						for( auto gradeNode : levelNode["Grades"] ){
							// Convert Refine + Chance to a Chances array
							if( gradeNode["Refine"].IsDefined() && !gradeNode["Chance"].IsDefined() ){
								ShowError( "Cannot upgrade automatically, because Refine is specified, but Chance is missing" );
								return false;
							}

							if( gradeNode["Chance"].IsDefined() && !gradeNode["Refine"].IsDefined() ){
								ShowError( "Cannot upgrade automatically, because Chance is specified, but Refine is missing" );
								return false;
							}

							uint16 refine = gradeNode["Refine"].as<uint16>();
							uint16 chance = gradeNode["Chance"].as<uint16>();

							auto chancesNode = gradeNode["Chances"];

							for( int32 i = refine, j = 0; i <= MAX_REFINE; i++, j++ ){
								auto chanceNode = chancesNode[j];

								chanceNode["Refine"] = i;
								chanceNode["Chance"] = chance;
							}

							// Remove the existing Refine entry
							gradeNode.remove( "Refine" );

							// Remove the existing Chance entry
							gradeNode.remove( "Chance" );
						}
					}
				}
			}
		}

		body << input;
		entries++;
	}

	ShowStatus( "Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str() );

	return true;
}

static bool upgrade_item_group_db( std::string file, const uint32 source_version ){
	size_t entries = 0;

	for( const auto input : inNode["Body"] ){
		if( source_version < 4 ){
			body << YAML::BeginMap;
			body << YAML::Key << "Group" << YAML::Value << input["Group"];

			if( input["SubGroups"].IsDefined() ){
				body << YAML::Key << "SubGroups";
				body << YAML::BeginSeq;

				for (const auto &it : input["SubGroups"]) {
					body << YAML::BeginMap;
					if( !it["SubGroup"].IsDefined() ){
						ShowError( "Cannot upgrade automatically, SubGroup is missing." );
						return false;
					}
					body << YAML::Key << "SubGroup" << YAML::Value << it["SubGroup"];

					if (it["SubGroup"].as<uint16>() == 0)
						body << YAML::Key << "Algorithm" << YAML::Value << "All";
					else if (it["SubGroup"].as<uint16>() == 6)
						body << YAML::Key << "Algorithm" << YAML::Value << "Random";
					// else
						// body << YAML::Key << "Algorithm" << YAML::Value << "SharedPool";

					if( it["List"].IsDefined() )
						body << YAML::Key << "List";{
						body << YAML::BeginSeq;

						uint32 index = 0;

						for( auto ListNode : it["List"] ){
							if( !ListNode["Item"].IsDefined() ){
								ShowError( "Cannot upgrade automatically, Item is missing" );
								return false;
							}
							body << YAML::BeginMap;

							body << YAML::Key << "Index" << YAML::Value << index;
							body << YAML::Key << "Item" << YAML::Value << ListNode["Item"];

							if( ListNode["Rate"].IsDefined() )
								body << YAML::Key << "Rate" << YAML::Value << ListNode["Rate"];
							if( ListNode["Amount"].IsDefined() )
								body << YAML::Key << "Amount" << YAML::Value << ListNode["Amount"];
							if( ListNode["Duration"].IsDefined() )
								body << YAML::Key << "Duration" << YAML::Value << ListNode["Duration"];
							if( ListNode["Announced"].IsDefined() )
								body << YAML::Key << "Announced" << YAML::Value << ListNode["Announced"];
							if( ListNode["UniqueId"].IsDefined() )
								body << YAML::Key << "UniqueId" << YAML::Value << ListNode["UniqueId"];
							if( ListNode["Stacked"].IsDefined() )
								body << YAML::Key << "Stacked" << YAML::Value << ListNode["Stacked"];
							if( ListNode["Named"].IsDefined() )
								body << YAML::Key << "Named" << YAML::Value << ListNode["Named"];
							if( ListNode["Bound"].IsDefined() )
								body << YAML::Key << "Bound" << YAML::Value << ListNode["Bound"];
							if( ListNode["RandomOptionGroup"].IsDefined() )
								body << YAML::Key << "RandomOptionGroup" << YAML::Value << ListNode["RandomOptionGroup"];
							if( ListNode["RefineMinimum"].IsDefined() )
								body << YAML::Key << "RefineMinimum" << YAML::Value << ListNode["RefineMinimum"];
							if( ListNode["RefineMaximum"].IsDefined() )
								body << YAML::Key << "RefineMaximum" << YAML::Value << ListNode["RefineMaximum"];
							if( ListNode["Clear"].IsDefined() )
								body << YAML::Key << "Clear" << YAML::Value << ListNode["Clear"];

							index++;
							body << YAML::EndMap;
						}

						body << YAML::EndSeq;
					}
					if( it["Clear"].IsDefined() )
						body << YAML::Key << "Clear" << YAML::Value << it["Clear"];

					body << YAML::EndMap;
				}
				body << YAML::EndSeq;
			}
			body << YAML::EndMap;
		}

		entries++;
	}

	ShowStatus( "Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str() );

	return true;
}

static bool upgrade_skill_db( std::string file, const uint32 source_version ){
	size_t entries = 0;

	for( auto input : inNode["Body"] ){
		// If under version 4
		if( source_version < 4 ){
			if( input["CastCancel"].IsDefined() ){
				// If CastCancel was true (new default value)
				if( input["CastCancel"].as<bool>() ){
					// Remove it
					input.remove( "CastCancel" );
				}
			}else{
				if( input["CastTime"].IsDefined() || input["FixedCastTime"].IsDefined() ){
					input.force_insert( "CastCancel", false );
				}
			}
		}

		body << input;
		entries++;
	}

	ShowStatus( "Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str() );

	return true;
}

int32 main( int32 argc, char *argv[] ){
	return main_core<YamlUpgradeTool>( argc, argv );
}
