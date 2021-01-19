// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "yamlupgrade.hpp"

template<typename Func>
bool process(const std::string &type, uint32 version, const std::vector<std::string> &paths, const std::string &name, Func lambda) {
	for (const std::string &path : paths) {
		const std::string name_ext = name + ".yml";
		const std::string from = path + "/" + name_ext;
		const std::string to = path + "/" + name + "-upgrade.yml";

		inNode.reset();
		inNode = YAML::LoadFile(from);

		if (fileExists(from) && getHeaderVersion(inNode) < version) {
			if (!askConfirmation("Found the file \"%s\", which requires an upgrade.\nDo you want to convert it now? (Y/N)\n", from.c_str())) {
				continue;
			}

			if (fileExists(to)) {
				if (!askConfirmation("The file \"%s\" already exists.\nDo you want to replace it? (Y/N)\n", to.c_str())) {
					continue;
				}
			}

			std::ofstream out;

			body.~Emitter();
			new (&body) YAML::Emitter();
			out.open(to);

			if (!out.is_open()) {
				ShowError("Can not open file \"%s\" for writing.\n", to.c_str());
				return false;
			}

			prepareHeader(out, type, version, name);
			if (inNode["Body"].IsDefined()) {
				prepareBody();

				if (!lambda(path, name_ext)) {
					out.close();
					return false;
				}

				finalizeBody();
				out << body.c_str();
			}
			prepareFooter(out);
			// Make sure there is an empty line at the end of the file for git
			out << "\n";
			out.close();
		}
	}

	return true;
}

int do_init(int argc, char** argv) {
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
	sv_readdb(path_db_mode.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants, false);
	sv_readdb(path_db_import.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants, false);
	if (fileExists(skill_db.getDefaultLocation())) {
		skill_db.load();
	} else {
		sv_readdb(path_db_mode.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants_txt, false);
		sv_readdb(path_db_import.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants_txt, false);
	}

	// Load constants
	#define export_constant_npc(a) export_constant(a)
	#include "../map/script_constants.hpp"

	std::vector<std::string> root_paths = {
		path_db,
		path_db_mode,
		path_db_import
	};

	if (!process("ACHIEVEMENT_DB", 2, root_paths, "achievement_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return upgrade_achievement_db(path + name_ext);
	})) {
		return 0;
	}

	return 0;
}

void do_final(void) {
}

// Implementation of the upgrade functions
static bool upgrade_achievement_db(std::string file) {
	size_t entries = 0;

	for (const auto &input : inNode["Body"]) {
		body << YAML::BeginMap;
		body << YAML::Key << "Id" << YAML::Value << input["ID"];

		std::string constant = input["Group"].as<std::string>();

		constant.erase(0, 3); // Remove "AG_"
		if (constant.compare("Hear") == 0 || constant.compare("See") == 0)
			constant = "Chatting"; // Aegis treats these as general "Talk to NPC" achievements.
		body << YAML::Key << "Group" << YAML::Value << name2Upper(constant);
		body << YAML::Key << "Name" << YAML::Value << input["Name"];

		if (input["Target"].IsDefined()) {
			body << YAML::Key << "Targets";
			body << YAML::BeginSeq;

			for (const auto &it : input["Target"]) {
				body << YAML::BeginMap;
				body << YAML::Key << "Id" << YAML::Value << it["Id"];
				if (it["MobID"].IsDefined())
					body << YAML::Key << "Mob" << YAML::Value << *util::umap_find(aegis_mobnames, it["MobID"].as<uint16>());
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

		if (input["Reward"].IsDefined()) {
			body << YAML::Key << "Rewards";
			body << YAML::BeginMap;
			if (input["Reward"]["ItemID"].IsDefined())
				body << YAML::Key << "Item" << YAML::Value << *util::umap_find(aegis_itemnames, input["Reward"]["ItemID"].as<t_itemid>());
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

	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' achievements in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}
