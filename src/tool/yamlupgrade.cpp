// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "yamlupgrade.hpp"

using namespace rathena::tool_yamlupgrade;
using ryml_tool::as;
using ryml_tool::defined;
using ryml_tool::child;
namespace emit = ryml_tool::emit;

static bool upgrade_achievement_db(std::string file, const uint32 source_version);
static bool upgrade_item_db(std::string file, const uint32 source_version);
static bool upgrade_job_stats(std::string file, const uint32 source_version);
static bool upgrade_status_db(std::string file, const uint32 source_version);
static bool upgrade_map_drops_db(std::string file, const uint32 source_version);
static bool upgrade_enchantgrade_db( std::string file, const uint32 source_version );
static bool upgrade_item_group_db( std::string file, const uint32 source_version );
static bool upgrade_skill_db( std::string file, const uint32 source_version );
static bool upgrade_item_packages_db( std::string file, const uint32 source_version );

template<typename Func>
bool process(const std::string &type, uint32 version, const std::vector<std::string> &paths, const std::string &name, Func lambda) {
	for (const std::string &path : paths) {
		const std::string name_ext = name + ".yml";
		const std::string from = path + "/" + name_ext;
		const std::string to = path + "/" + name + "-upgrade.yml";

		inTree.clear();

		if (fileExists(from)) {
			try {
				ryml_tool::load_tree_from_file(from, inParser, inTree);
			} catch (const std::runtime_error& e) {
				ShowError("%s\n", e.what());
				continue;
			}

			uint32 source_version = getHeaderVersion(inTree.rootref());

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

			body.~RymlEmitter();
			new (&body) RymlEmitter();
			outFile.open(to);

			if (!outFile.is_open()) {
				ShowError("Can not open file \"%s\" for writing.\n", to.c_str());
				return false;
			}

			prepareHeader(outFile, type, version, name);
			if (defined(child(inTree.rootref(), "Body"))) {
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
	if( !process( "ITEM_GROUP_DB", 5, root_paths, "item_group_db", []( const std::string& path, const std::string& name_ext, uint32 source_version ) -> bool {
		return upgrade_item_group_db( path + name_ext, source_version );
		} ) ){
		return false;
	}

	if( !process( "SKILL_DB", 4, root_paths, "skill_db", []( const std::string& path, const std::string& name_ext, uint32 source_version ) -> bool {
		return upgrade_skill_db( path + name_ext, source_version );
		} ) ){
		return false;
	}

	if( !process( "ITEM_PACKAGE_DB", 2, root_paths, "item_packages", []( const std::string& path, const std::string& name_ext, uint32 source_version ) -> bool {
		return upgrade_item_packages_db( path + name_ext, source_version );
		} ) ){
		return false;
	}

	return true;
}

// Implementation of the upgrade functions
static bool upgrade_achievement_db(std::string file, const uint32 source_version) {
	size_t entries = 0;
	ryml::NodeRef bodyNode = child(inTree.rootref(), "Body");

	for (ryml::NodeRef input : bodyNode.children()) {
		body << emit::BeginMap;
		body << emit::Key << "Id" << emit::Value << input["ID"];

		std::string constant = as<std::string>(input["Group"]);

		constant.erase(0, 3); // Remove "AG_"
		if (constant.compare("Chat") == 0) // Chat -> Chatting
			constant.insert(4, "ting");
		else if (constant.compare("Hear") == 0 || constant.compare("See") == 0)
			constant = "Chatting"; // Aegis treats these as general "Talk to NPC" achievements.
		else if (constant.compare("Refine") == 0) { // Refine -> Enchant
			constant.erase(0, 6);
			constant = "Enchant" + constant;
		}
		body << emit::Key << "Group" << emit::Value << name2Upper(constant);
		body << emit::Key << "Name" << emit::Value << input["Name"];

		if (defined(input["Target"])) {
			body << emit::Key << "Targets";
			body << emit::BeginSeq;

			for (ryml::NodeRef it : input["Target"].children()) {
				body << emit::BeginMap;
				body << emit::Key << "Id" << emit::Value << it["Id"];
				if (defined(it["MobID"])) {
					uint16 mob_id = as<uint16>(it["MobID"]);
					std::string *mob_name = util::umap_find(aegis_mobnames, mob_id);

					if (mob_name == nullptr) {
						ShowWarning("mob_avail reading: Invalid mob-class %hu, Mob not read.\n", mob_id);
						return false;
					}

					body << emit::Key << "Mob" << emit::Value << *mob_name;
				}
				if (defined(it["Count"]) && as<int32>(it["Count"]) > 1)
					body << emit::Key << "Count" << emit::Value << it["Count"];
				body << emit::EndMap;
			}

			body << emit::EndSeq;
		}

		if (defined(input["Condition"]))
			body << emit::Key << "Condition" << emit::Value << input["Condition"];

		if (defined(input["Map"]))
			body << emit::Key << "Map" << emit::Value << input["Map"];

		if (defined(input["Dependent"])) {
			body << emit::Key << "Dependents";
			body << emit::BeginMap;

			for (ryml::NodeRef it : input["Dependent"].children()) {
				body << emit::Key << it["Id"] << emit::Value << true;
			}

			body << emit::EndMap;
		}

		/**
		 * Example usage for adding label at specific version.
		if (source_version < ?) {
			body << ryml_tool::emit::Key << "CustomLabel" << ryml_tool::emit::Value << "Unique";
		}
		*/

		if (defined(input["Reward"])) {
			body << emit::Key << "Rewards";
			body << emit::BeginMap;
			if (defined(input["Reward"]["ItemID"])) {
				t_itemid item_id = as<t_itemid>(input["Reward"]["ItemID"]);
				std::string *item_name = util::umap_find(aegis_itemnames, item_id);

				if (item_name == nullptr) {
					ShowError("Reward item name for item ID %u is not known.\n", item_id);
					return false;
				}

				body << emit::Key << "Item" << emit::Value << *item_name;
			}
			if (defined(input["Reward"]["Amount"]) && as<uint16>(input["Reward"]["Amount"]) > 1)
				body << emit::Key << "Amount" << emit::Value << input["Reward"]["Amount"];
			if (defined(input["Reward"]["Script"]))
				body << emit::Key << "Script" << emit::Value << input["Reward"]["Script"];
			if (defined(input["Reward"]["TitleID"]))
				body << emit::Key << "TitleId" << emit::Value << input["Reward"]["TitleID"];
			body << emit::EndMap;
		}

		body << emit::Key << "Score" << emit::Value << input["Score"];

		body << emit::EndMap;
		entries++;
	}

	ShowStatus("Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' achievements in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}

static bool upgrade_item_db(std::string file, const uint32 source_version) {
	size_t entries = 0;
	ryml::NodeRef bodyNode = child(inTree.rootref(), "Body");

	for (ryml::NodeRef input : bodyNode.children()) {
		// If under version 2
		if( source_version < 2 ){
			// Add armor level to all equipments
			if( defined(input["Type"]) && as<std::string>(input["Type"]) == "Armor" ){
				input["ArmorLevel"] << 1;
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
	ryml::NodeRef bodyNode = child(inTree.rootref(), "Body");

	for (ryml::NodeRef input : bodyNode.children()) {
		// If under version 2
		if (source_version < 2) {
			// Field name changes
			if (defined(input["HPFactor"])) {
				input["HpFactor"] << as<uint32>(input["HPFactor"]);
				input.remove_child("HPFactor");
			}
			if (defined(input["HpMultiplicator"])) {
				input["HpIncrease"] << as<uint32>(input["HpMultiplicator"]);
				input.remove_child("HpMultiplicator");
			}
			if (defined(input["HPMultiplicator"])) {
				input["HpIncrease"] << as<uint32>(input["HPMultiplicator"]);
				input.remove_child("HPMultiplicator");
			}
			if (defined(input["SpFactor"])) {
				input["SpIncrease"] << as<uint32>(input["SpFactor"]);
				input.remove_child("SpFactor");
			}
			if (defined(input["SPFactor"])) {
				input["SpIncrease"] << as<uint32>(input["SPFactor"]);
				input.remove_child("SPFactor");
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
	ryml::NodeRef bodyNode = child(inTree.rootref(), "Body");

	for (ryml::NodeRef input : bodyNode.children()) {
		// If under version 3
		if (source_version < 3) {
			// Rename End to EndOnStart
			if (defined(input["End"])) {
				input["EndOnStart"] << ryml_tool::scalar(input["End"]);
				input.remove_child("End");
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
	ryml::NodeRef bodyNode = child(inTree.rootref(), "Body");

	for (ryml::NodeRef input : bodyNode.children()) {
		// If under version 2, adjust the rates from n/10000 to n/100000
		if( source_version < 2 ){
			if (defined(input["GlobalDrops"])) {
				for (ryml::NodeRef GlobalDrops : input["GlobalDrops"].children()) {
					if (defined(GlobalDrops["Rate"])) {
						uint32 val = as<uint32>(GlobalDrops["Rate"]) * 10;
						GlobalDrops["Rate"] << val;
					}
				}
			}
			if (defined(input["SpecificDrops"])) {
				for (ryml::NodeRef SpecificDrops : input["SpecificDrops"].children()) {
					if (defined(SpecificDrops["Drops"])) {
						for (ryml::NodeRef Drops : SpecificDrops["Drops"].children()) {
							if (defined(Drops["Rate"])) {
								uint32 val = as<uint32>(Drops["Rate"]) * 10;
								Drops["Rate"] << val;
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
	ryml::NodeRef bodyNode = child(inTree.rootref(), "Body");

	for (ryml::NodeRef input : bodyNode.children()) {
		// If under version 3
		if( source_version < 3 ){
			if (defined(input["Levels"])) {
				for (ryml::NodeRef levelNode : input["Levels"].children()) {
					if (defined(levelNode["Grades"])) {
						for (ryml::NodeRef gradeNode : levelNode["Grades"].children()) {
							// Convert Refine + Chance to a Chances array
							if (defined(gradeNode["Refine"]) && !defined(gradeNode["Chance"])) {
								ShowError( "Cannot upgrade automatically, because Refine is specified, but Chance is missing" );
								return false;
							}

							if (defined(gradeNode["Chance"]) && !defined(gradeNode["Refine"])) {
								ShowError( "Cannot upgrade automatically, because Chance is specified, but Refine is missing" );
								return false;
							}

							uint16 refine = as<uint16>(gradeNode["Refine"]);
							uint16 chance = as<uint16>(gradeNode["Chance"]);

							auto chancesNode = gradeNode["Chances"];
							chancesNode = ryml::SEQ;
							chancesNode |= ryml::_WIP_STYLE_BLOCK;

							for( int32 i = refine, j = 0; i <= MAX_REFINE; i++, j++ ){
								auto chanceNode = chancesNode[j];

								chanceNode = ryml::MAP;
								chanceNode |= ryml::_WIP_STYLE_BLOCK;
								chanceNode["Refine"] << i;
								chanceNode["Chance"] << chance;
							}

							// Remove the existing Refine entry
							gradeNode.remove_child("Refine");

							// Remove the existing Chance entry
							gradeNode.remove_child("Chance");
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
	ryml::NodeRef bodyNode = child(inTree.rootref(), "Body");

	for (ryml::NodeRef input : bodyNode.children()) {
		// Added the "Algorithm" field based on the previous "Subgroup"
		if( source_version < 4 ){
			body << emit::BeginMap;
			body << emit::Key << "Group" << emit::Value << input["Group"];

			if (defined(input["SubGroups"])) {
				body << emit::Key << "SubGroups";
				body << emit::BeginSeq;

				for (ryml::NodeRef it : input["SubGroups"].children()) {
					body << emit::BeginMap;
					if (!defined(it["SubGroup"])) {
						ShowError( "Cannot upgrade automatically, SubGroup is missing." );
						return false;
					}
					body << emit::Key << "SubGroup" << emit::Value << it["SubGroup"];

					if (as<uint16>(it["SubGroup"]) == 0)
						body << emit::Key << "Algorithm" << emit::Value << "All";
					else if (as<uint16>(it["SubGroup"]) == 6)
						body << emit::Key << "Algorithm" << emit::Value << "Random";
					// else
						// body << ryml_tool::emit::Key << "Algorithm" << ryml_tool::emit::Value << "SharedPool";

					if (defined(it["List"])) {
						body << emit::Key << "List";
						body << emit::BeginSeq;

						uint32 index = 0;

						for (ryml::NodeRef ListNode : it["List"].children()) {
							if (!defined(ListNode["Item"])) {
								ShowError( "Cannot upgrade automatically, Item is missing" );
								return false;
							}
							body << emit::BeginMap;

							body << emit::Key << "Index" << emit::Value << index;
							body << emit::Key << "Item" << emit::Value << ListNode["Item"];

							if (defined(ListNode["Rate"]))
								body << emit::Key << "Rate" << emit::Value << ListNode["Rate"];
							if (defined(ListNode["Amount"]))
								body << emit::Key << "Amount" << emit::Value << ListNode["Amount"];
							if (defined(ListNode["Duration"]))
								body << emit::Key << "Duration" << emit::Value << ListNode["Duration"];
							if (defined(ListNode["Announced"]))
								body << emit::Key << "Announced" << emit::Value << ListNode["Announced"];
							if (defined(ListNode["UniqueId"]))
								body << emit::Key << "UniqueId" << emit::Value << ListNode["UniqueId"];
							if (defined(ListNode["Stacked"]))
								body << emit::Key << "Stacked" << emit::Value << ListNode["Stacked"];
							if (defined(ListNode["Named"]))
								body << emit::Key << "Named" << emit::Value << ListNode["Named"];
							if (defined(ListNode["Bound"]))
								body << emit::Key << "Bound" << emit::Value << ListNode["Bound"];
							if (defined(ListNode["RandomOptionGroup"]))
								body << emit::Key << "RandomOptionGroup" << emit::Value << ListNode["RandomOptionGroup"];
							if (defined(ListNode["RefineMinimum"]))
								body << emit::Key << "RefineMinimum" << emit::Value << ListNode["RefineMinimum"];
							if (defined(ListNode["RefineMaximum"]))
								body << emit::Key << "RefineMaximum" << emit::Value << ListNode["RefineMaximum"];
							if (defined(ListNode["Clear"]))
								body << emit::Key << "Clear" << emit::Value << ListNode["Clear"];

							index++;
							body << emit::EndMap;
						}

						body << emit::EndSeq;
					}
					if (defined(it["Clear"]))
						body << emit::Key << "Clear" << emit::Value << it["Clear"];

					body << emit::EndMap;
				}
				body << emit::EndSeq;
			}
			body << emit::EndMap;
		}

		entries++;
	}

	ShowStatus( "Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str() );

	return true;
}

static bool upgrade_skill_db( std::string file, const uint32 source_version ){
	size_t entries = 0;
	ryml::NodeRef bodyNode = child(inTree.rootref(), "Body");

	for (ryml::NodeRef input : bodyNode.children()) {
		// If under version 4
		if( source_version < 4 ){
			if (defined(input["CastCancel"])) {
				// If CastCancel was true (new default value)
				if (as<bool>(input["CastCancel"])) {
					// Remove it
					input.remove_child("CastCancel");
				}
			}else{
				if (defined(input["CastTime"]) || defined(input["FixedCastTime"])) {
					if (defined(input["CastCancel"])) {
						input.remove_child("CastCancel");
					}
					input["CastCancel"] << false;
				}
			}
		}

		body << input;
		entries++;
	}

	ShowStatus( "Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str() );

	return true;
}

static bool upgrade_item_packages_db( std::string file, const uint32 source_version ){
	size_t entries = 0;
	ryml::NodeRef bodyNode = child(inTree.rootref(), "Body");

	for (ryml::NodeRef input : bodyNode.children()) {
		body << input;
		entries++;
	}

	ShowStatus( "Done converting/upgrading '" CL_WHITE "%zu" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str() );

	return true;
}

int32 main( int32 argc, char *argv[] ){
	return main_core<YamlUpgradeTool>( argc, argv );
}
