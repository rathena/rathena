// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "csv2yaml.hpp"

#include <cmath>

using namespace rathena::tool_csv2yaml;

// Skill database data to memory
static void skill_txt_data(const std::string& modePath, const std::string& fixedPath) {
	skill_require.clear();
	skill_cast.clear();
	skill_castnodex.clear();
	skill_unit.clear();
	skill_copyable.clear();
	skill_nearnpc.clear();

	if (fileExists(modePath + "/skill_require_db.txt"))
		sv_readdb(modePath.c_str(), "skill_require_db.txt", ',', 34, 34, -1, skill_parse_row_requiredb, false);
	if (fileExists(modePath + "/skill_cast_db.txt"))
		sv_readdb(modePath.c_str(), "skill_cast_db.txt", ',', 7, 8, -1, skill_parse_row_castdb, false);
	if (fileExists(modePath + "/skill_castnodex_db.txt"))
		sv_readdb(modePath.c_str(), "skill_castnodex_db.txt", ',', 2, 3, -1, skill_parse_row_castnodexdb, false);
	if (fileExists(modePath + "/skill_unit_db.txt"))
		sv_readdb(modePath.c_str(), "skill_unit_db.txt", ',', 8, 8, -1, skill_parse_row_unitdb, false);
	if (fileExists(fixedPath + "/skill_copyable_db.txt"))
		sv_readdb(fixedPath.c_str(), "skill_copyable_db.txt", ',', 2, 4, -1, skill_parse_row_copyabledb, false);
	if (fileExists(fixedPath + "/skill_nonearnpc_db.txt"))
		sv_readdb(fixedPath.c_str(), "skill_nonearnpc_db.txt", ',', 2, 3, -1, skill_parse_row_nonearnpcrangedb, false);
}

// Item database data to memory
static void item_txt_data(const std::string& modePath, const std::string& fixedPath) {
	item_avail.clear();
	item_buyingstore.clear();
	item_flag.clear();
	item_delay.clear();
	item_stack.clear();
	item_nouse.clear();
	item_trade.clear();

	if (fileExists(fixedPath + "/item_avail.txt"))
		sv_readdb(fixedPath.c_str(), "item_avail.txt", ',', 2, 2, -1, &itemdb_read_itemavail, false);
	if (fileExists(modePath + "/item_buyingstore.txt"))
		sv_readdb(modePath.c_str(), "item_buyingstore.txt", ',', 1, 1, -1, &itemdb_read_buyingstore, false);
	if (fileExists(modePath + "/item_flag.txt"))
		sv_readdb(modePath.c_str(), "item_flag.txt", ',', 2, 2, -1, &itemdb_read_flag, false);
	if (fileExists(modePath + "/item_delay.txt"))
		sv_readdb(modePath.c_str(), "item_delay.txt", ',', 2, 3, -1, &itemdb_read_itemdelay, false);
	if (fileExists(modePath + "/item_stack.txt"))
		sv_readdb(modePath.c_str(), "item_stack.txt", ',', 3, 3, -1, &itemdb_read_stack, false);
	if (fileExists(fixedPath + "/item_nouse.txt"))
		sv_readdb(fixedPath.c_str(), "item_nouse.txt", ',', 3, 3, -1, &itemdb_read_nouse, false);
	if (fileExists(modePath + "/item_trade.txt"))
		sv_readdb(modePath.c_str(), "item_trade.txt", ',', 3, 3, -1, &itemdb_read_itemtrade, false);
}

// Homunculus database data to memory
static void homunculus_txt_data(const std::string& modePath, const std::string& fixedPath) {
	hom_skill_tree.clear();

	if (fileExists(modePath + "/homun_skill_tree.txt"))
		sv_readdb(modePath.c_str(), "homun_skill_tree.txt", ',', 15, 15, -1, read_homunculus_skilldb, false);
}

// Mob database data to memory
static void mob_txt_data(const std::string &modePath, const std::string &fixedPath) {
	mob_race2.clear();
	mob_drop.clear();

	if (fileExists(modePath + "/mob_race2_db.txt"))
		sv_readdb(modePath.c_str(), "mob_race2_db.txt", ',', 2, 100, -1, mob_readdb_race2, false);
	if (fileExists(modePath + "/mob_drop.txt"))
		sv_readdb(modePath.c_str(), "mob_drop.txt", ',', 3, 5, -1, mob_readdb_drop, false);
}

// Branch database data to memory
static void branch_txt_data(const std::string& modePath, const std::string& fixedPath) {
	summon_group.clear();

	if (fileExists(modePath + "/mob_branch.txt"))
		sv_readdb(modePath.c_str(), "mob_branch.txt", ',', 4, 4, -1, mob_readdb_group, false);
	if (fileExists(modePath + "/mob_random_db.txt"))
		sv_readdb(modePath.c_str(), "mob_random_db.txt", ',', 4, 4, -1, mob_readdb_group, false);
	if (fileExists(modePath + "/mob_poring.txt"))
		sv_readdb(modePath.c_str(), "mob_poring.txt", ',', 4, 4, -1, mob_readdb_group, false);
	if (fileExists(modePath + "/mob_boss.txt"))
		sv_readdb(modePath.c_str(), "mob_boss.txt", ',', 4, 4, -1, mob_readdb_group, false);
	if (fileExists(fixedPath + "/mob_pouch.txt"))
		sv_readdb(fixedPath.c_str(), "mob_pouch.txt", ',', 4, 4, -1, mob_readdb_group, false);
	if (fileExists(fixedPath + "/mob_mission.txt"))
		sv_readdb(fixedPath.c_str(), "mob_mission.txt", ',', 4, 4, -1, mob_readdb_group, false);
	if (fileExists(fixedPath + "/mob_classchange.txt"))
		sv_readdb(fixedPath.c_str(), "mob_classchange.txt", ',', 4, 4, -1, mob_readdb_group, false);
}

// Item Group database data to memory
static void item_group_txt_data(const std::string& modePath, const std::string& fixedPath) {
	item_group.clear();

	if (fileExists(modePath + "/item_bluebox.txt"))
		sv_readdb(modePath.c_str(), "item_bluebox.txt", ',', 2, 10, -1, itemdb_read_group, false);
	if (fileExists(modePath + "/item_violetbox.txt"))
		sv_readdb(modePath.c_str(), "item_violetbox.txt", ',', 2, 10, -1, itemdb_read_group, false);
	if (fileExists(modePath + "/item_cardalbum.txt"))
		sv_readdb(modePath.c_str(), "item_cardalbum.txt", ',', 2, 10, -1, itemdb_read_group, false);
	if (fileExists(fixedPath + "/item_findingore.txt"))
		sv_readdb(fixedPath.c_str(), "item_findingore.txt", ',', 2, 10, -1, itemdb_read_group, false);
	if (fileExists(modePath + "/item_giftbox.txt"))
		sv_readdb(modePath.c_str(), "item_giftbox.txt", ',', 2, 10, -1, itemdb_read_group, false);
	if (fileExists(modePath + "/item_misc.txt"))
		sv_readdb(modePath.c_str(), "item_misc.txt", ',', 2, 10, -1, itemdb_read_group, false);
	if (fileExists(modePath + "/item_group_db.txt"))
		sv_readdb(modePath.c_str(), "item_group_db.txt", ',', 2, 10, -1, itemdb_read_group, false);
	if (fileExists(modePath + "/item_package.txt"))
		sv_readdb(modePath.c_str(), "item_package.txt", ',', 2, 10, -1, itemdb_read_group, false);
}

// Job database data to memory
static void job_txt_data(const std::string &modePath, const std::string &fixedPath) {
	job_db2.clear();
	job_param.clear();
	exp_base_level.clear();
	exp_job_level.clear();

	if (fileExists(fixedPath + "/job_db2.txt"))
		sv_readdb(fixedPath.c_str(), "/job_db2.txt", ',', 1, 1 + MAX_LEVEL, CLASS_COUNT, &pc_readdb_job2, false);
	if (fileExists(modePath + "job_exp.txt"))
		sv_readdb(modePath.c_str(), "job_exp.txt", ',', 4, 1000 + 3, CLASS_COUNT * 2, &pc_readdb_job_exp_sub, false);
	if (fileExists(modePath + "job_param_db.txt"))
		sv_readdb(modePath.c_str(), "job_param_db.txt", ',', 2, PARAM_MAX + 1, CLASS_COUNT, &pc_readdb_job_param, false);
}

// Elemental Summons Skill Database data to memory
static void elemental_skill_txt_data(const std::string& modePath, const std::string& fixedPath) {
	elemental_skill_tree.clear();

	if (fileExists(fixedPath + "/elemental_skill_db.txt"))
		sv_readdb(fixedPath.c_str(), "elemental_skill_db.txt", ',', 4, 4, -1, read_elemental_skilldb, false);
}

// Mercenary's Skill Database data to memory
static void mercenary_skill_txt_data(const std::string& modePath, const std::string& fixedPath) {
	mercenary_skill_tree.clear();

	if (fileExists(fixedPath + "/mercenary_skill_db.txt"))
		sv_readdb(fixedPath.c_str(), "mercenary_skill_db.txt", ',', 3, 3, -1, mercenary_read_skilldb, false);
}

// Skill tree database data to memory
static void skilltree_txt_data(const std::string &modePath, const std::string &fixedPath) {
	skill_tree.clear();

	if (fileExists(modePath + "/skill_tree.txt"))
		sv_readdb(modePath.c_str(), "skill_tree.txt", ',', 3 + MAX_PC_SKILL_REQUIRE * 2, 5 + MAX_PC_SKILL_REQUIRE * 2, -1, pc_readdb_skilltree, false);
}

template<typename Func>
bool process( const std::string& type, uint32 version, const std::vector<std::string>& paths, const std::string& name, Func lambda, const std::string& rename = "" ){
	for( const std::string& path : paths ){
		const std::string name_ext = name + ".txt";
		const std::string from = path + "/" + name_ext;
		const std::string to = path + "/" + (rename.size() > 0 ? rename : name) + ".yml";

		if( fileExists( from ) ){
#ifndef CONVERT_ALL
			if( !askConfirmation( "Found the file \"%s\", which requires migration to yml.\nDo you want to convert it now? (Y/N)\n", from.c_str() ) ){
				continue;
			}

			if (fileExists(to)) {
				if (!askConfirmation("The file \"%s\" already exists.\nDo you want to replace it? (Y/N)\n", to.c_str())) {
					continue;
				}
			}
#else
			ShowMessage( "Found the file \"%s\", which requires migration to yml.\n", from.c_str() );
#endif

			ShowNotice("Conversion process has begun.\n");

			std::ofstream outFile;

			body.~Emitter();
			new (&body) YAML::Emitter();
			outFile.open(to);

			if (!outFile.is_open()) {
				ShowError("Can not open file \"%s\" for writing.\n", to.c_str());
				return false;
			}

			prepareHeader(outFile, type, version, (rename.size() > 0 ? rename : name));
			prepareBody();

			if( !lambda( path, name_ext ) ){
				outFile.close();
				return false;
			}

			finalizeBody();
			outFile << body.c_str();
			// Make sure there is an empty line at the end of the file for git
			outFile << "\n";
			outFile.close();
			
			// TODO: do you want to delete?
		}
	}

	return true;
}

bool Csv2YamlTool::initialize( int32 argc, char* argv[] ){
	const std::string path_db = std::string( db_path );
	const std::string path_db_mode = path_db + "/" + DBPATH;
	const std::string path_db_import = path_db + "/" + DBIMPORT + "/";

	// Loads required conversion constants
	if (fileExists(item_db.getDefaultLocation())) {
		item_db.load();
	} else {
		parse_item_constants_txt( ( path_db_mode + "item_db.txt" ).c_str() );
		parse_item_constants_txt( ( path_db_import + "item_db.txt" ).c_str() );
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
	init_random_option_constants();
	#include <map/script_constants.hpp>
	// Constants that are deprecated but still needed for conversion
	script_set_constant(QUOTE(RC2_GUARDIAN), RC2_GUARDIAN, false, false);
	script_set_constant(QUOTE(RC2_BATTLEFIELD), RC2_BATTLEFIELD, false, false);

	std::vector<std::string> root_paths = {
		path_db,
		path_db_mode,
		path_db_import
	};

	if( !process( "GUILD_SKILL_TREE_DB", 1, root_paths, "guild_skill_tree", []( const std::string& path, const std::string& name_ext ) -> bool {
		return sv_readdb( path.c_str(), name_ext.c_str(), ',', 2 + MAX_GUILD_SKILL_REQUIRE * 2, 2 + MAX_GUILD_SKILL_REQUIRE * 2, -1, &guild_read_guildskill_tree_db, false );
	} ) ){
		return false;
	}

	if( !process( "PET_DB", 1, root_paths, "pet_db", []( const std::string& path, const std::string& name_ext ) -> bool {
		return pet_read_db( ( path + name_ext ).c_str() );
	} ) ){
		return false;
	}

	if (!process("MAGIC_MUSHROOM_DB", 1, root_paths, "magicmushroom_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 1, 1, -1, &skill_parse_row_magicmushroomdb, false);
	})) {
		return false;
	}

	if (!process("ABRA_DB", 1, root_paths, "abra_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 3, 3, -1, &skill_parse_row_abradb, false);
	})) {
		return false;
	}

	if (!process("READING_SPELLBOOK_DB", 1, root_paths, "spellbook_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 3, 3, -1, &skill_parse_row_spellbookdb, false);
	})) {
		return false;
	}

	if (!process("MOB_AVAIL_DB", 1, root_paths, "mob_avail", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 2, 12, -1, &mob_readdb_mobavail, false);
	})) {
		return false;
	}

	skill_txt_data( path_db_mode, path_db );
	if (!process("SKILL_DB", 4, { path_db_mode }, "skill_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 18, 18, -1, &skill_parse_row_skilldb, false);
	})){
		return false;
	}

	skill_txt_data( path_db_import, path_db_import );
	if (!process("SKILL_DB", 4, { path_db_import }, "skill_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 18, 18, -1, &skill_parse_row_skilldb, false);
	})){
		return false;
	}

	if (!process("QUEST_DB", 3, root_paths, "quest_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 3 + MAX_QUEST_OBJECTIVES * 2 + MAX_QUEST_DROPS * 3, 100, -1, &quest_read_db, false);
	})) {
		return false;
	}

	if (!process("INSTANCE_DB", 1, root_paths, "instance_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 7, 7 + MAX_MAP_PER_INSTANCE, -1, &instance_readdb_sub, false);
	})) {
		return false;
	}

	item_txt_data(path_db_mode, path_db);
	if (!process("ITEM_DB", 3, { path_db_mode }, "item_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return itemdb_read_db((path + name_ext).c_str());
	})) {
		return false;
	}

	item_txt_data(path_db_import, path_db_import);
	if (!process("ITEM_DB", 3, { path_db_import }, "item_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return itemdb_read_db((path + name_ext).c_str());
	})) {
		return false;
	}

	rand_opt_db.clear();
	if (!process("RANDOM_OPTION_DB", 1, root_paths, "item_randomopt_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return itemdb_read_randomopt((path + name_ext).c_str());
	})) {
		return false;
	}

	rand_opt_group.clear();
	if (!process("RANDOM_OPTION_GROUP", 1, root_paths, "item_randomopt_group", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 5, 2 + 5 * MAX_ITEM_RDM_OPT, -1, &itemdb_read_randomopt_group, false) && itemdb_randomopt_group_yaml();
	})) {
		return false;
	}

#ifdef RENEWAL
	memset( level_penalty, 0, sizeof( level_penalty ) );
	if (!process("PENALTY_DB", 1, { path_db_mode }, "level_penalty", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 4, 4, -1, &pc_readdb_levelpenalty, false) && pc_levelpenalty_yaml();
	})) {
		return false;
	}

	memset( level_penalty, 0, sizeof( level_penalty ) );
	if (!process("PENALTY_DB", 1, { path_db_import }, "level_penalty", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 4, 4, -1, &pc_readdb_levelpenalty, false) && pc_levelpenalty_yaml();
	})) {
		return false;
	}
#endif

	mob_txt_data(path_db_mode, path_db);
	if (!process("MOB_DB", 3, { path_db_mode }, "mob_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &mob_readdb_sub, false);
	})) {
		return false;
	}

	mob_txt_data(path_db_import, path_db_import);
	if (!process("MOB_DB", 3, { path_db_import }, "mob_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &mob_readdb_sub, false);
	})) {
		return false;
	}

	if (!process("MOB_CHAT_DB", 1, root_paths, "mob_chat_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), '#', 3, 3, -1, &mob_parse_row_chatdb, false);
	})) {
		return false;
	}

	if (!process("HOMUN_EXP_DB", 1, { path_db_mode }, "exp_homun", [](const std::string &path, const std::string &name_ext) -> bool {
		return read_homunculus_expdb((path + name_ext).c_str());
	})) {
		return false;
	}

	if (!process("HOMUN_EXP_DB", 1, { path_db_import }, "exp_homun", [](const std::string &path, const std::string &name_ext) -> bool {
		return read_homunculus_expdb((path + name_ext).c_str());
	})) {
		return false;
	}

	branch_txt_data(path_db_mode, path_db);
	if (!process("MOB_SUMMONABLE_DB", 1, { path_db_mode }, "mob_branch", [](const std::string &path, const std::string &name_ext) -> bool {
		return mob_readdb_group_yaml();
	}, "mob_summon")) {
		return false;
	}

	branch_txt_data(path_db_import, path_db_import);
	if (!process("MOB_SUMMONABLE_DB", 1, { path_db_import }, "mob_branch", [](const std::string &path, const std::string &name_ext) -> bool {
		return mob_readdb_group_yaml();
	}, "mob_summon")) {
		return false;
	}

	if (!process("CREATE_ARROW_DB", 1, root_paths, "create_arrow_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 1+2, 1+2*MAX_ARROW_RESULT, MAX_SKILL_ARROW_DB, &skill_parse_row_createarrowdb, false);
	})) {
		return false;
	}

	if (!process("STATPOINT_DB", 1, { path_db_mode }, "statpoint", [](const std::string &path, const std::string &name_ext) -> bool {
		return pc_read_statsdb((path + name_ext).c_str());
	})) {
		return false;
	}

	if (!process("STATPOINT_DB", 1, { path_db_import }, "statpoint", [](const std::string &path, const std::string &name_ext) -> bool {
		return pc_read_statsdb((path + name_ext).c_str());
	})) {
		return false;
	}

	if (!process("CASTLE_DB", 1, root_paths, "castle_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 4, 4, -1, &guild_read_castledb, false);
	})) {
		return false;
	}

	if (!process("GUILD_EXP_DB", 1, { path_db_mode }, "exp_guild", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 1, 1, MAX_GUILDLEVEL, &exp_guild_parse_row, false);
	})) {
		return false;
	}

	if (!process("GUILD_EXP_DB", 1, { path_db_import }, "exp_guild", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 1, 1, MAX_GUILDLEVEL, &exp_guild_parse_row, false);
	})) {
		return false;
	}

	item_group_txt_data(path_db_mode, path_db);
	if (!process("ITEM_GROUP_DB", 4, { path_db_mode }, "item_group_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return itemdb_read_group_yaml();
	})) {
		return false;
	}

	item_group_txt_data(path_db_import, path_db_import);
	if (!process("ITEM_GROUP_DB", 4, { path_db_import }, "item_group_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return itemdb_read_group_yaml();
	})) {
		return false;
	}

	if (!process("MOB_ITEM_RATIO_DB", 1, root_paths, "mob_item_ratio", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 2, 2+MAX_ITEMRATIO_MOBS, -1, &mob_readdb_itemratio, false);
	})) {
		return false;
	}

	if (!process("ATTRIBUTE_DB", 1, { path_db_mode }, "attr_fix", [](const std::string &path, const std::string &name_ext) -> bool {
		return status_readdb_attrfix((path + name_ext).c_str());
	})) {
		return false;
	}

	if (!process("ATTRIBUTE_DB", 1, { path_db_import }, "attr_fix", [](const std::string &path, const std::string &name_ext) -> bool {
		return status_readdb_attrfix((path + name_ext).c_str());
	})) {
		return false;
	}

	if (!process("CONSTANT_DB", 1, root_paths, "const", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 1, 3, -1, &read_constdb, false);
	})) {
		return false;
	}

	if (!process("JOB_STATS", 3, root_paths, "job_exp", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 4, 1000 + 3, CLASS_COUNT * 2, &pc_readdb_job_exp, false);
	}, "job_exp")) {
		return false;
	}

	if (!process("JOB_STATS", 3, root_paths, "job_basehpsp_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 4, 4 + 500, CLASS_COUNT * 2, &pc_readdb_job_basehpsp, false);
	}, "job_basepoints")) {
		return false;
	}

	job_txt_data(path_db_mode, path_db);
	if (!process("JOB_STATS", 3, { path_db_mode }, "job_db1", [](const std::string& path, const std::string& name_ext) -> bool {
#ifdef RENEWAL_ASPD
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 6 + MAX_WEAPON_TYPE, 6 + MAX_WEAPON_TYPE, CLASS_COUNT, &pc_readdb_job1, false);
#else
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 5 + MAX_WEAPON_TYPE, 5 + MAX_WEAPON_TYPE, CLASS_COUNT, &pc_readdb_job1, false);
#endif
	}, "job_stats")) {
		return false;
	}

	job_txt_data(path_db_import, path_db_import);
	if (!process("JOB_STATS", 3, { path_db_import }, "job_db1", [](const std::string& path, const std::string& name_ext) -> bool {
#ifdef RENEWAL_ASPD
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 6 + MAX_WEAPON_TYPE, 6 + MAX_WEAPON_TYPE, CLASS_COUNT, &pc_readdb_job1, false);
#else
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 5 + MAX_WEAPON_TYPE, 5 + MAX_WEAPON_TYPE, CLASS_COUNT, &pc_readdb_job1, false);
#endif
	}, "job_stats")) {
		return false;
	}

	elemental_skill_txt_data(path_db_mode, path_db);
	if (!process("ELEMENTAL_DB", 1, root_paths, "elemental_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 26, 26, -1, &read_elementaldb, false);
	})) {
		return false;
	}

	mercenary_skill_txt_data(path_db_mode, path_db);
	if (!process("MERCENARY_DB", 1, root_paths, "mercenary_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 26, 26, -1, &mercenary_readdb, false);
	})) {
		return false;
	}

	skilltree_txt_data(path_db_mode, path_db);
	if (!process("SKILL_TREE_DB", 1, { path_db_mode }, "skill_tree", [](const std::string &path, const std::string &name_ext) -> bool {
		return pc_readdb_skilltree_yaml();
	})) {
		return false;
	}

	skilltree_txt_data(path_db_import, path_db_import);
	if (!process("SKILL_TREE_DB", 1, { path_db_import }, "skill_tree", [](const std::string &path, const std::string &name_ext) -> bool {
		return pc_readdb_skilltree_yaml();
	})) {
		return false;
	}

	if (!process("COMBO_DB", 1, { path_db_mode }, "item_combo_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return itemdb_read_combos((path + name_ext).c_str());
	}, "item_combos")) {
		return false;
	}

	if (!process("COMBO_DB", 1, { path_db_import }, "item_combo_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return itemdb_read_combos((path + name_ext).c_str());
	}, "item_combos")) {
		return false;
	}

	if( !process( "ITEM_CASH_DB", 1, root_paths, "item_cash_db", []( const std::string& path, const std::string& name_ext ) -> bool {
		return sv_readdb( path.c_str(), name_ext.c_str(), ',', 3, 3, -1, &cashshop_parse_dbrow, false );
	}, "item_cash" ) ){
		return 0;
	}

	homunculus_txt_data(path_db, path_db);
	if (!process("HOMUNCULUS_DB", 1, { path_db_mode }, "homunculus_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 50, 50, MAX_HOMUNCULUS_CLASS, read_homunculusdb, false);
	})) {
		return 0;
	}
	
	homunculus_txt_data(path_db_import, path_db_import);
	if (!process("HOMUNCULUS_DB", 1, { path_db_import }, "homunculus_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 50, 50, MAX_HOMUNCULUS_CLASS, read_homunculusdb, false);
	})) {
		return 0;
	}

	// TODO: add implementations ;-)

	return true;
}

// Implementation of the conversion functions

// Copied and adjusted from guild.cpp
// <skill id>,<max lv>,<req id1>,<req lv1>,<req id2>,<req lv2>,<req id3>,<req lv3>,<req id4>,<req lv4>,<req id5>,<req lv5>
static bool guild_read_guildskill_tree_db( char* split[], size_t columns, size_t current ){
	uint16 skill_id = (uint16)atoi(split[0]);
	std::string* name = util::umap_find( aegis_skillnames, skill_id );

	if( name == nullptr ){
		ShowError( "Skill name for skill id %hu is not known.\n", skill_id );
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << *name;
	body << YAML::Key << "MaxLevel" << YAML::Value << atoi(split[1]);

	if (atoi(split[2]) > 0) {
		body << YAML::Key << "Required";
		body << YAML::BeginSeq;

		for (int32 i = 0, j = 0; i < MAX_GUILD_SKILL_REQUIRE; i++) {
			uint16 required_skill_id = atoi(split[i * 2 + 2]);
			uint16 required_skill_level = atoi(split[i * 2 + 3]);

			if (required_skill_id == 0 || required_skill_level == 0) {
				continue;
			}

			std::string* required_name = util::umap_find(aegis_skillnames, required_skill_id);

			if (required_name == nullptr) {
				ShowError("Skill name for required skill id %hu is not known.\n", required_skill_id);
				return false;
			}

			body << YAML::BeginMap;
			body << YAML::Key << "Id" << YAML::Value << *required_name;
			body << YAML::Key << "Level" << YAML::Value << required_skill_level;
			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from pet.cpp
static bool pet_read_db( const char* file ){
	FILE* fp = fopen( file, "r" );

	if( fp == nullptr ){
		ShowError( "can't read %s\n", file );
		return false;
	}

	int32 lines = 0;
	size_t entries = 0;
	char line[1024];

	while( fgets( line, sizeof(line), fp ) ) {
		char *str[22], *p;
		unsigned k;
		lines++;

		if(line[0] == '/' && line[1] == '/')
			continue;

		memset(str, 0, sizeof(str));
		p = line;

		while( ISSPACE(*p) )
			++p;

		if( *p == '\0' )
			continue; // empty line

		for( k = 0; k < 20; ++k ) {
			str[k] = p;
			p = strchr(p,',');

			if( p == nullptr )
				break; // comma not found

			*p = '\0';
			++p;
		}

		if( p == nullptr ) {
			ShowError("read_petdb: Insufficient columns in line %d, skipping.\n", lines);
			continue;
		}

		// Pet Script
		if( *p != '{' ) {
			ShowError("read_petdb: Invalid format (Pet Script column) in line %d, skipping.\n", lines);
			continue;
		}

		str[20] = p;
		p = strstr(p+1,"},");

		if( p == nullptr ) {
			ShowError("read_petdb: Invalid format (Pet Script column) in line %d, skipping.\n", lines);
			continue;
		}

		p[1] = '\0';
		p += 2;

		// Equip Script
		if( *p != '{' ) {
			ShowError("read_petdb: Invalid format (Equip Script column) in line %d, skipping.\n", lines);
			continue;
		}

		str[21] = p;

		uint16 mob_id = atoi( str[0] );
		std::string* mob_name = util::umap_find( aegis_mobnames, mob_id );

		if( mob_name == nullptr ){
			ShowWarning( "pet_db reading: Invalid mob-class %hu, pet not read.\n", mob_id );
			continue;
		}

		body << YAML::BeginMap;
		body << YAML::Key << "Mob" << YAML::Value << *mob_name;

		t_itemid tame_item_id = strtoul( str[3], nullptr, 10 );

		if( tame_item_id > 0 ){
			std::string* tame_item_name = util::umap_find( aegis_itemnames, tame_item_id );

			if( tame_item_name == nullptr ){
				ShowError( "Item name for item id %u is not known.\n", tame_item_id );
				return false;
			}

			body << YAML::Key << "TameItem" << YAML::Value << *tame_item_name;
		}

		t_itemid egg_item_id = strtoul( str[4], nullptr, 10 );
		std::string* egg_item_name = util::umap_find( aegis_itemnames, egg_item_id );

		if( egg_item_name == nullptr ){
			ShowError( "Item name for item id %u is not known.\n", egg_item_id );
			return false;
		}

		body << YAML::Key << "EggItem" << YAML::Value << *egg_item_name;

		t_itemid equip_item_id = strtoul( str[5], nullptr, 10 );

		if( equip_item_id > 0 ){
			std::string* equip_item_name = util::umap_find( aegis_itemnames, equip_item_id );

			if( equip_item_name == nullptr ){
				ShowError( "Item name for item id %u is not known.\n", equip_item_id );
				return false;
			}

			body << YAML::Key << "EquipItem" << YAML::Value << *equip_item_name;
		}

		t_itemid food_item_id = strtoul( str[6], nullptr, 10 );

		if( food_item_id > 0 ){
			std::string* food_item_name = util::umap_find( aegis_itemnames, food_item_id );

			if( food_item_name == nullptr ){
				ShowError( "Item name for item id %u is not known.\n", food_item_id );
				return false;
			}

			body << YAML::Key << "FoodItem" << YAML::Value << *food_item_name;
		}

		body << YAML::Key << "Fullness" << YAML::Value << atoi(str[7]);
		// Default: 60
		if( atoi( str[8] ) != 60 ){
			body << YAML::Key << "HungryDelay" << YAML::Value << atoi(str[8]);
		}
		// Default: 250
		if( atoi( str[11] ) != 250 ){
			body << YAML::Key << "IntimacyStart" << YAML::Value << atoi(str[11]);
		}
		body << YAML::Key << "IntimacyFed" << YAML::Value << atoi(str[9]);
		// Default: -100
		if( atoi( str[10] ) != 100 ){
			body << YAML::Key << "IntimacyOverfed" << YAML::Value << -atoi(str[10]);
		}
		// pet_hungry_friendly_decrease battle_conf
		//body << YAML::Key << "IntimacyHungry" << YAML::Value << -5;
		// Default: -20
		if( atoi( str[12] ) != 20 ){
			body << YAML::Key << "IntimacyOwnerDie" << YAML::Value << -atoi(str[12]);
		}
		body << YAML::Key << "CaptureRate" << YAML::Value << atoi(str[13]);
		// Default: true
		if( atoi( str[15] ) == 0 ){
			body << YAML::Key << "SpecialPerformance" << YAML::Value << "false";
		}
		body << YAML::Key << "AttackRate" << YAML::Value << atoi(str[17]);
		body << YAML::Key << "RetaliateRate" << YAML::Value << atoi(str[18]);
		body << YAML::Key << "ChangeTargetRate" << YAML::Value << atoi(str[19]);

		if( *str[21] ){
			body << YAML::Key << "Script" << YAML::Value << YAML::Literal << str[21];
		}

		if( *str[20] ){
			body << YAML::Key << "SupportScript" << YAML::Value << YAML::Literal << str[20];
		}

		body << YAML::EndMap;
		entries++;
	}

	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%zu" CL_RESET "' pets in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file );

	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_magicmushroomdb( char *split[], size_t column, size_t current ){
	uint16 skill_id = atoi(split[0]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("Skill name for Magic Mushroom skill ID %hu is not known.\n", skill_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Skill" << YAML::Value << *skill_name;
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_abradb( char* split[], size_t columns, size_t current ){
	uint16 skill_id = atoi(split[0]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("Skill name for Abra skill ID %hu is not known.\n", skill_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Skill" << YAML::Value << *skill_name;

	int32 arr[MAX_SKILL_LEVEL];
	int32 arr_size = skill_split_atoi(split[2], arr);

	if (arr_size == 1) {
		if (arr[0] != 500)
			body << YAML::Key << "Probability" << YAML::Value << arr[0];
	} else {
		body << YAML::Key << "Probability";
		body << YAML::BeginSeq;

		for (int32 i = 0; i < arr_size; i++) {
			if (arr[i] > 0) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Probability" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}
		}

		body << YAML::EndSeq;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_spellbookdb( char* split[], size_t columns, size_t current ){
	uint16 skill_id = atoi(split[0]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("Skill name for Spell Book skill ID %hu is not known.\n", skill_id);
		return false;
	}

	t_itemid nameid = strtoul(split[2], nullptr, 10);
	std::string *book_name = util::umap_find(aegis_itemnames, nameid);

	if (book_name == nullptr) {
		ShowError("Book name for item ID %u is not known.\n", nameid);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Skill" << YAML::Value << *skill_name;
	body << YAML::Key << "Book" << YAML::Value << *book_name;
	body << YAML::Key << "PreservePoints" << YAML::Value << atoi(split[1]);
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from mob.cpp
static bool mob_readdb_mobavail( char* str[], size_t columns, size_t current ){
	uint16 mob_id = atoi(str[0]);
	std::string *mob_name = util::umap_find(aegis_mobnames, mob_id);

	if (mob_name == nullptr) {
		ShowWarning("mob_avail reading: Invalid mob-class %hu, Mob not read.\n", mob_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Mob" << YAML::Value << *mob_name;

	uint16 sprite_id = atoi(str[1]);
	std::string *sprite_name = util::umap_find(aegis_mobnames, sprite_id);

	if (sprite_name == nullptr) {
		char *sprite = const_cast<char *>(constant_lookup(sprite_id, "JOB_"));

		if (sprite == nullptr) {
			sprite = const_cast<char *>(constant_lookup(sprite_id, "JT_"));

			if (sprite == nullptr) {
				ShowError("Sprite name for id %d is not known.\n", sprite_id);
				return false;
			}

			sprite += 3; // Strip JT_ here because the script engine doesn't send this prefix for NPC.

			body << YAML::Key << "Sprite" << YAML::Value << sprite;
		} else
			body << YAML::Key << "Sprite" << YAML::Value << sprite;
	} else
		body << YAML::Key << "Sprite" << YAML::Value << *sprite_name;

	if (columns == 12) {
		body << YAML::Key << "Sex" << YAML::Value << (atoi(str[2]) ? "Male" : "Female");
		if (atoi(str[3]) != 0)
			body << YAML::Key << "HairStyle" << YAML::Value << atoi(str[3]);
		if (atoi(str[4]) != 0)
			body << YAML::Key << "HairColor" << YAML::Value << atoi(str[4]);
		if (atoi(str[11]) != 0)
			body << YAML::Key << "ClothColor" << YAML::Value << atoi(str[11]);

		if (strtoul(str[5], nullptr, 10) != 0) {
			t_itemid weapon_item_id = strtoul( str[5], nullptr, 10 );
			std::string *weapon_item_name = util::umap_find(aegis_itemnames, weapon_item_id);

			if (weapon_item_name == nullptr) {
				ShowError("Item name for item ID %u (weapon) is not known.\n", weapon_item_id);
				return false;
			}

			body << YAML::Key << "Weapon" << YAML::Value << *weapon_item_name;
		}

		if (strtoul(str[6], nullptr, 10) != 0) {
			t_itemid shield_item_id = strtoul( str[6], nullptr, 10 );
			std::string *shield_item_name = util::umap_find(aegis_itemnames, shield_item_id);

			if (shield_item_name == nullptr) {
				ShowError("Item name for item ID %u (shield) is not known.\n", shield_item_id);
				return false;
			}

			body << YAML::Key << "Shield" << YAML::Value << *shield_item_name;
		}

		if (strtoul(str[7], nullptr, 10) != 0) {
			t_itemid *headtop_item_id = util::umap_find(aegis_itemviewid, (uint32)strtoul(str[7], nullptr, 10));

			if (headtop_item_id == nullptr) {
				ShowError("Item ID for view ID %lu (head top) is not known.\n", strtoul(str[7], nullptr, 10));
				return false;
			}

			std::string *headtop_item_name = util::umap_find(aegis_itemnames, *headtop_item_id);

			if (headtop_item_name == nullptr) {
				ShowError("Item name for item ID %u (head top) is not known.\n", *headtop_item_id);
				return false;
			}

			body << YAML::Key << "HeadTop" << YAML::Value << *headtop_item_name;
		}

		if (strtoul(str[8], nullptr, 10) != 0) {
			t_itemid *headmid_item_id = util::umap_find(aegis_itemviewid, (uint32)strtoul(str[8], nullptr, 10));

			if (headmid_item_id == nullptr) {
				ShowError("Item ID for view ID %lu (head mid) is not known.\n", strtoul(str[8], nullptr, 10));
				return false;
			}

			std::string *headmid_item_name = util::umap_find(aegis_itemnames, *headmid_item_id);

			if (headmid_item_name == nullptr) {
				ShowError("Item name for item ID %u (head mid) is not known.\n", *headmid_item_id);
				return false;
			}

			body << YAML::Key << "HeadMid" << YAML::Value << *headmid_item_name;
		}

		if (strtoul(str[9], nullptr, 10) != 0) {
			t_itemid *headlow_item_id = util::umap_find(aegis_itemviewid, (uint32)strtoul(str[9], nullptr, 10));

			if (headlow_item_id == nullptr) {
				ShowError("Item ID for view ID %lu (head low) is not known.\n", strtoul(str[9], nullptr, 10));
				return false;
			}

			std::string *headlow_item_name = util::umap_find(aegis_itemnames, *headlow_item_id);

			if (headlow_item_name == nullptr) {
				ShowError("Item name for item ID %u (head low) is not known.\n", *headlow_item_id);
				return false;
			}

			body << YAML::Key << "HeadLow" << YAML::Value << *headlow_item_name;
		}

		if (atoi(str[10]) != 0) {
			uint32 options = atoi(str[10]);

			body << YAML::Key << "Options";
			body << YAML::BeginMap;

			while (options > OPTION_NOTHING && options <= OPTION_SUMMER2) {
				if (options & OPTION_SIGHT) {
					body << YAML::Key << "Sight" << YAML::Value << "true";
					options &= ~OPTION_SIGHT;
				} else if (options & OPTION_CART1) {
					body << YAML::Key << "Cart1" << YAML::Value << "true";
					options &= ~OPTION_CART1;
				} else if (options & OPTION_FALCON) {
					body << YAML::Key << "Falcon" << YAML::Value << "true";
					options &= ~OPTION_FALCON;
				} else if (options & OPTION_RIDING) {
					body << YAML::Key << "Riding" << YAML::Value << "true";
					options &= ~OPTION_RIDING;
				} else if (options & OPTION_CART2) {
					body << YAML::Key << "Cart2" << YAML::Value << "true";
					options &= ~OPTION_CART2;
				} else if (options & OPTION_CART3) {
					body << YAML::Key << "Cart2" << YAML::Value << "true";
					options &= ~OPTION_CART3;
				} else if (options & OPTION_CART4) {
					body << YAML::Key << "Cart4" << YAML::Value << "true";
					options &= ~OPTION_CART4;
				} else if (options & OPTION_CART5) {
					body << YAML::Key << "Cart5" << YAML::Value << "true";
					options &= ~OPTION_CART5;
				} else if (options & OPTION_ORCISH) {
					body << YAML::Key << "Orcish" << YAML::Value << "true";
					options &= ~OPTION_ORCISH;
				} else if (options & OPTION_WEDDING) {
					body << YAML::Key << "Wedding" << YAML::Value << "true";
					options &= ~OPTION_WEDDING;
				} else if (options & OPTION_RUWACH) {
					body << YAML::Key << "Ruwach" << YAML::Value << "true";
					options &= ~OPTION_RUWACH;
				} else if (options & OPTION_FLYING) {
					body << YAML::Key << "Flying" << YAML::Value << "true";
					options &= ~OPTION_FLYING;
				} else if (options & OPTION_XMAS) {
					body << YAML::Key << "Xmas" << YAML::Value << "true";
					options &= ~OPTION_XMAS;
				} else if (options & OPTION_TRANSFORM) {
					body << YAML::Key << "Transform" << YAML::Value << "true";
					options &= ~OPTION_TRANSFORM;
				} else if (options & OPTION_SUMMER) {
					body << YAML::Key << "Summer" << YAML::Value << "true";
					options &= ~OPTION_SUMMER;
				} else if (options & OPTION_DRAGON1) {
					body << YAML::Key << "Dragon1" << YAML::Value << "true";
					options &= ~OPTION_DRAGON1;
				} else if (options & OPTION_WUG) {
					body << YAML::Key << "Wug" << YAML::Value << "true";
					options &= ~OPTION_WUG;
				} else if (options & OPTION_WUGRIDER) {
					body << YAML::Key << "WugRider" << YAML::Value << "true";
					options &= ~OPTION_WUGRIDER;
				} else if (options & OPTION_MADOGEAR) {
					body << YAML::Key << "MadoGear" << YAML::Value << "true";
					options &= ~OPTION_MADOGEAR;
				} else if (options & OPTION_DRAGON2) {
					body << YAML::Key << "Dragon2" << YAML::Value << "true";
					options &= ~OPTION_DRAGON2;
				} else if (options & OPTION_DRAGON3) {
					body << YAML::Key << "Dragon3" << YAML::Value << "true";
					options &= ~OPTION_DRAGON3;
				} else if (options & OPTION_DRAGON4) {
					body << YAML::Key << "Dragon4" << YAML::Value << "true";
					options &= ~OPTION_DRAGON4;
				} else if (options & OPTION_DRAGON5) {
					body << YAML::Key << "Dragon5" << YAML::Value << "true";
					options &= ~OPTION_DRAGON5;
				} else if (options & OPTION_HANBOK) {
					body << YAML::Key << "Hanbok" << YAML::Value << "true";
					options &= ~OPTION_HANBOK;
				} else if (options & OPTION_OKTOBERFEST) {
					body << YAML::Key << "Oktoberfest" << YAML::Value << "true";
					options &= ~OPTION_OKTOBERFEST;
				} else if (options & OPTION_SUMMER2) {
					body << YAML::Key << "Summer2" << YAML::Value << "true";
					options &= ~OPTION_SUMMER2;
				}
			}

			body << YAML::EndMap;
		}
	} else if (columns == 3) {
		if (atoi(str[5]) != 0) {
			t_itemid peteq_item_id = strtoul( str[5], nullptr, 10 );
			std::string *peteq_item_name = util::umap_find(aegis_itemnames, peteq_item_id);

			if (peteq_item_name == nullptr) {
				ShowError("Item name for item ID %u (pet equip) is not known.\n", peteq_item_id);
				return false;
			}

			body << YAML::Key << "PetEquip" << YAML::Value << *peteq_item_name;
		}
	}

	body << YAML::EndMap;

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_requiredb( char* split[], size_t columns, size_t current ){
	s_skill_require entry = {};

	skill_split_atoi(split[1], entry.hp);
	skill_split_atoi(split[2], entry.mhp);
	skill_split_atoi(split[3], entry.sp);
	skill_split_atoi(split[4], entry.hp_rate);
	skill_split_atoi(split[5], entry.sp_rate);
	skill_split_atoi(split[6], entry.zeny);

	char *p;

	p = split[7];
	while (p) {
		int32 l = atoi(p);

		if (l == 99) { // Any weapon
			entry.weapon = 0;
			break;
		} else
			entry.weapon |= 1 << l;
		p = strchr(p, ':');
		if (!p)
			break;
		p++;
	}

	p = split[8];
	while (p) {
		int32 l = atoi(p);

		if (l == 99) { // Any ammo type
			entry.ammo = AMMO_TYPE_ALL;
			break;
		} else if (l) // 0 stands for no requirement
			entry.ammo |= 1 << l;
		p = strchr(p, ':');
		if (!p)
			break;
		p++;
	}

	skill_split_atoi(split[9], entry.ammo_qty);

	if (strcmpi(split[10], "hidden") == 0) entry.state = ST_HIDDEN;
	else if (strcmpi(split[10], "riding") == 0) entry.state = ST_RIDING;
	else if (strcmpi(split[10], "falcon") == 0) entry.state = ST_FALCON;
	else if (strcmpi(split[10], "cart") == 0) entry.state = ST_CART;
	else if (strcmpi(split[10], "shield") == 0) entry.state = ST_SHIELD;
	else if (strcmpi(split[10], "recover_weight_rate") == 0) entry.state = ST_RECOVER_WEIGHT_RATE;
	else if (strcmpi(split[10], "move_enable") == 0) entry.state = ST_MOVE_ENABLE;
	else if (strcmpi(split[10], "water") == 0) entry.state = ST_WATER;
	else if (strcmpi(split[10], "dragon") == 0) entry.state = ST_RIDINGDRAGON;
	else if (strcmpi(split[10], "warg") == 0) entry.state = ST_WUG;
	else if (strcmpi(split[10], "ridingwarg") == 0) entry.state = ST_RIDINGWUG;
	else if (strcmpi(split[10], "mado") == 0) entry.state = ST_MADO;
	else if (strcmpi(split[10], "elementalspirit") == 0) entry.state = ST_ELEMENTALSPIRIT;
	else if (strcmpi(split[10], "elementalspirit2") == 0) entry.state = ST_ELEMENTALSPIRIT2;
	else if (strcmpi(split[10], "peco") == 0) entry.state = ST_PECO;
	else if (strcmpi(split[10], "sunstance") == 0) entry.state = ST_SUNSTANCE;
	else if (strcmpi(split[10], "moonstance") == 0) entry.state = ST_MOONSTANCE;
	else if (strcmpi(split[10], "starstance") == 0) entry.state = ST_STARSTANCE;
	else if (strcmpi(split[10], "universestance") == 0) entry.state = ST_UNIVERSESTANCE;
	else entry.state = ST_NONE;	// Unknown or no state

	trim(split[11]);
	if (split[11][0] != '\0' || atoi(split[11])) {
		int64 require[MAX_SKILL_STATUS_REQUIRE];
		int32 count;

		if ((count = skill_split_atoi2(split[11], require, ":", SC_STONE, ARRAYLENGTH(require)))) {
			for (int32 i = 0; i < count; i++) {
				entry.status.push_back((sc_type)require[i]);
			}
		}
	}

	skill_split_atoi(split[12], entry.spiritball);

	for (int32 i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
		if (atoi(split[13 + 2 * i]) > 0) {
			t_itemid item_id = strtoul( split[13 + 2 * i], nullptr, 10 );
			std::string *item_name = util::umap_find(aegis_itemnames, item_id);

			if (item_name == nullptr) {
				ShowError("Item name for item id %u is not known.\n", item_id);
				return false;
			}

			entry.itemid[i] = item_id;
			entry.amount[i] = atoi(split[14 + 2 * i]);
		}
	}

	trim(split[33]);
	if (split[33][0] != '\0' || atoi(split[33])) {
		int64 require[MAX_SKILL_EQUIP_REQUIRE];
		int32 count;

		if ((count = skill_split_atoi2(split[33], require, ":", 500, ARRAYLENGTH(require)))) {
			for (int32 i = 0; i < count; i++) {
				if (require[i] > 0)
					entry.eqItem.push_back(static_cast<int32>(require[i]));
			}
		}
	}

	skill_require.insert({ atoi(split[0]), entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_castdb( char* split[], size_t columns, size_t current ){
	s_skill_db entry = {};

	skill_split_atoi(split[1], entry.cast);
	skill_split_atoi(split[2], entry.delay);
	skill_split_atoi(split[3], entry.walkdelay);
	skill_split_atoi(split[4], entry.upkeep_time);
	skill_split_atoi(split[5], entry.upkeep_time2);
	skill_split_atoi(split[6], entry.cooldown);
#ifdef RENEWAL_CAST
	skill_split_atoi(split[7], (int32 *)entry.fixed_cast);
#endif

	skill_cast.insert({atoi(split[0]), std::move(entry)});

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_castnodexdb( char* split[], size_t columns, size_t current ){
	s_skill_db entry = {};

	entry.castnodex = atoi(split[1]);
	if (split[2]) // optional column
		entry.delaynodex = atoi(split[2]);

	skill_castnodex.insert({atoi(split[0]), std::move(entry)});

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_unitdb( char* split[], size_t columns, size_t current ){
	s_skill_unit_csv entry = {};

	entry.unit_id = (uint16)strtol(split[1], nullptr, 16);
	entry.unit_id2 = (uint16)strtol(split[2], nullptr, 16);
	skill_split_atoi(split[3], entry.unit_layout_type);
	skill_split_atoi(split[4], entry.unit_range);
	entry.unit_interval = atoi(split[5]);
	entry.target_str = trim(split[6]);
	entry.unit_flag_csv = strtol(split[7], nullptr, 16);

	skill_unit.insert({atoi(split[0]), std::move(entry)});

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_copyabledb( char* split[], size_t column, size_t current ){
	s_skill_copyable entry = {};
	int32 skill_id = -1;

	trim(split[0]);
	if (ISDIGIT(split[0][0]))
		skill_id = atoi(split[0]);
	else {
		for (const auto &it : aegis_skillnames) {
			if (it.second == split[0])
				skill_id = it.first;
		}

		if (skill_id == -1) {
			ShowError("Skill %s is unknown.\n", split[0]);
			return false;
		}
	}

	entry.option = atoi(split[1]);
	entry.req_opt = cap_value(atoi(split[3]), 0, (0x2000) - 1);

	skill_copyable.insert({ skill_id, entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_nonearnpcrangedb( char* split[], size_t column, size_t current ){
	s_skill_db entry = {};
	int32 skill_id = -1;

	trim(split[0]);
	if (ISDIGIT(split[0][0]))
		skill_id = atoi(split[0]);
	else {
		for (const auto &it : aegis_skillnames) {
			if (it.second == split[0])
				skill_id = it.first;
		}

		if (skill_id == -1) {
			ShowError("Skill %s is unknown.\n", split[0]);
			return false;
		}
	}

	entry.unit_nonearnpc_range = max(atoi(split[1]), 0);
	entry.unit_nonearnpc_type = (atoi(split[2])) ? cap_value(atoi(split[2]), 1, 15) : 15;

	skill_nearnpc.insert({skill_id, std::move(entry)});

	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_skilldb( char* split[], size_t columns, size_t current ){
	int32 arr[MAX_SKILL_LEVEL], arr_size, skill_id = atoi(split[0]);

	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << skill_id;
	body << YAML::Key << "Name" << YAML::Value << trim(split[16]);
	body << YAML::Key << "Description" << YAML::Value << trim(split[17]);
	body << YAML::Key << "MaxLevel" << YAML::Value << atoi(split[7]);

	if (strcmpi(split[13], "weapon") == 0)
		body << YAML::Key << "Type" << YAML::Value << "Weapon";
	else if (strcmpi(split[13], "magic") == 0)
		body << YAML::Key << "Type" << YAML::Value << "Magic";
	else if (strcmpi(split[13], "misc") == 0)
		body << YAML::Key << "Type" << YAML::Value << "Misc";

	std::string constant;

	if (atoi(split[3]) != 0) {
		constant = constant_lookup(atoi(split[3]), "INF_");
		constant.erase(0, 4);
		constant.erase(constant.size() - 6);
		body << YAML::Key << "TargetType" << YAML::Value << name2Upper(constant);
	}

	uint64 nk_val = strtol(split[5], nullptr, 0);

	if (nk_val) {
		body << YAML::Key << "DamageFlags";
		body << YAML::BeginMap;
		if (nk_val & 0x1)
			body << YAML::Key << "NoDamage" << YAML::Value << "true";
		if (nk_val & 0x2)
			body << YAML::Key << "Splash" << YAML::Value << "true";
		if (nk_val & 0x4)
			body << YAML::Key << "SplashSplit" << YAML::Value << "true";
		if (nk_val & 0x8)
			body << YAML::Key << "IgnoreAtkCard" << YAML::Value << "true";
		if (nk_val & 0x10)
			body << YAML::Key << "IgnoreElement" << YAML::Value << "true";
		if (nk_val & 0x20)
			body << YAML::Key << "IgnoreDefense" << YAML::Value << "true";
		if (nk_val & 0x40)
			body << YAML::Key << "IgnoreFlee" << YAML::Value << "true";
		if (nk_val & 0x80)
			body << YAML::Key << "IgnoreDefCard" << YAML::Value << "true";
		if (nk_val & 0x100)
			body << YAML::Key << "Critical" << YAML::Value << "true";

		body << YAML::EndMap;
	}

	uint64 inf2_val = strtol(split[11], nullptr, 0);
	uint64 inf3_val = strtol(split[15], nullptr, 0);

	if (inf2_val || inf3_val) {
		body << YAML::Key << "Flags";
		body << YAML::BeginMap;
		if (inf2_val & 0x1)
			body << YAML::Key << "IsQuest" << YAML::Value << "true";
		if (inf2_val & 0x2)
			body << YAML::Key << "IsNpc" << YAML::Value << "true";
		if (inf2_val & 0x4)
			body << YAML::Key << "IsWedding" << YAML::Value << "true";
		if (inf2_val & 0x8)
			body << YAML::Key << "IsSpirit" << YAML::Value << "true";
		if (inf2_val & 0x10)
			body << YAML::Key << "IsGuild" << YAML::Value << "true";
		if (inf2_val & 0x20)
			body << YAML::Key << "IsSong" << YAML::Value << "true";
		if (inf2_val & 0x40)
			body << YAML::Key << "IsEnsemble" << YAML::Value << "true";
		if (inf2_val & 0x80)
			body << YAML::Key << "IsTrap" << YAML::Value << "true";
		if (inf2_val & 0x100)
			body << YAML::Key << "TargetSelf" << YAML::Value << "true";
		if (inf2_val & 0x200)
			body << YAML::Key << "NoTargetSelf" << YAML::Value << "true";
		if (inf2_val & 0x400)
			body << YAML::Key << "PartyOnly" << YAML::Value << "true";
		if (inf2_val & 0x800)
			body << YAML::Key << "GuildOnly" << YAML::Value << "true";
		if (inf2_val & 0x1000)
			body << YAML::Key << "NoTargetEnemy" << YAML::Value << "true";
		if (inf2_val & 0x2000)
			body << YAML::Key << "IsAutoShadowSpell" << YAML::Value << "true";
		if (inf2_val & 0x4000)
			body << YAML::Key << "IsChorus" << YAML::Value << "true";
		if (inf2_val & 0x8000)
			body << YAML::Key << "IgnoreBgReduction" << YAML::Value << "true";
		if (inf2_val & 0x10000)
			body << YAML::Key << "IgnoreGvgReduction" << YAML::Value << "true";
		if (inf2_val & 0x20000)
			body << YAML::Key << "DisableNearNpc" << YAML::Value << "true";
		if (inf2_val & 0x40000)
			body << YAML::Key << "TargetTrap" << YAML::Value << "true"; // ?

		if (inf3_val & 0x1)
			body << YAML::Key << "IgnoreLandProtector" << YAML::Value << "true";
		if (inf3_val & 0x4)
			body << YAML::Key << "AllowWhenHidden" << YAML::Value << "true";
		if (inf3_val & 0x8)
			body << YAML::Key << "AllowWhenPerforming" << YAML::Value << "true";
		if (inf3_val & 0x10)
			body << YAML::Key << "TargetEmperium" << YAML::Value << "true";
		if (inf3_val & 0x40)
			body << YAML::Key << "IgnoreKagehumi" << YAML::Value << "true";
		if (inf3_val & 0x80)
			body << YAML::Key << "AlterRangeVulture" << YAML::Value << "true";
		if (inf3_val & 0x100)
			body << YAML::Key << "AlterRangeSnakeEye" << YAML::Value << "true";
		if (inf3_val & 0x200)
			body << YAML::Key << "AlterRangeShadowJump" << YAML::Value << "true";
		if (inf3_val & 0x400)
			body << YAML::Key << "AlterRangeRadius" << YAML::Value << "true";
		if (inf3_val & 0x800)
			body << YAML::Key << "AlterRangeResearchTrap" << YAML::Value << "true";
		if (inf3_val & 0x1000)
			body << YAML::Key << "IgnoreHovering" << YAML::Value << "true";
		if (inf3_val & 0x2000)
			body << YAML::Key << "AllowOnWarg" << YAML::Value << "true";
		if (inf3_val & 0x4000)
			body << YAML::Key << "AllowOnMado" << YAML::Value << "true";
		if (inf3_val & 0x8000)
			body << YAML::Key << "TargetManHole" << YAML::Value << "true";
		if (inf3_val & 0x10000)
			body << YAML::Key << "TargetHidden" << YAML::Value << "true";
		if (inf3_val & 0x40000)
			body << YAML::Key << "IncreaseDanceWithWugDamage" << YAML::Value << "true";
		if (inf3_val & 0x80000)
			body << YAML::Key << "IgnoreWugBite" << YAML::Value << "true";
		if (inf3_val & 0x100000)
			body << YAML::Key << "IgnoreAutoGuard" << YAML::Value << "true";
		if (inf3_val & 0x200000)
			body << YAML::Key << "IgnoreCicada" << YAML::Value << "true";

		body << YAML::EndMap;
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[1], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "Range";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "Range";
			body << YAML::BeginSeq;

			for (int32 i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Size" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	if (atoi(split[2]) != 0) {
		constant = constant_lookup(atoi(split[2]), "DMG_");
		constant.erase(0, 4);
		body << YAML::Key << "Hit" << YAML::Value << name2Upper(constant);
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[8], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "HitCount";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "HitCount";
			body << YAML::BeginSeq;

			for (int32 i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Count" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[4], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0){
				body << YAML::Key << "Element";

				if (arr[0] == -1)
					body << YAML::Value << "Weapon";
				else if (arr[0] == -2)
					body << YAML::Value << "Endowed";
				else if (arr[0] == -3)
					body << YAML::Value << "Random";
				else {
					constant = constant_lookup(arr[0], "ELE_");
					constant.erase(0, 4);
					body << YAML::Value << name2Upper(constant);
				}
			}
		} else {
			body << YAML::Key << "Element";
			body << YAML::BeginSeq;

			for (int32 i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				if (arr[i] == -1)
					body << YAML::Key << "Element" << YAML::Value << "Weapon";
				else if (arr[i] == -2)
					body << YAML::Key << "Element" << YAML::Value << "Endowed";
				else if (arr[i] == -3)
					body << YAML::Key << "Element" << YAML::Value << "Random";
				else {
					constant = constant_lookup(arr[i], "ELE_");
					constant.erase(0, 4);
					body << YAML::Key << "Element" << YAML::Value << name2Upper(constant);
				}
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[6], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "SplashArea";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "SplashArea";
			body << YAML::BeginSeq;

			for (int32 i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Area" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[12], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "ActiveInstance";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "ActiveInstance";
			body << YAML::BeginSeq;

			for (int32 i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Max" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	memset(arr, 0, sizeof(arr));
	arr_size = skill_split_atoi(split[14], arr);

	if (arr_size != 0) {
		if (arr_size == 1) {
			if (arr[0] != 0) {
				body << YAML::Key << "Knockback";
				body << YAML::Value << arr[0];
			}
		} else {
			body << YAML::Key << "Knockback";
			body << YAML::BeginSeq;

			for (int32 i = 0; i < arr_size; i++) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Amount" << YAML::Value << arr[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
	}

	auto it_copyable = skill_copyable.find(skill_id);

	if (it_copyable != skill_copyable.end()) {
		body << YAML::Key << "CopyFlags";
		body << YAML::BeginMap;
		body << YAML::Key << "Skill";
		body << YAML::BeginMap;
		if (it_copyable->second.option & 1)
			body << YAML::Key << "Plagiarism" << YAML::Value << "true";
		if (it_copyable->second.option & 2)
			body << YAML::Key << "Reproduce" << YAML::Value << "true";
		body << YAML::EndMap;

		if (it_copyable->second.req_opt > 0) {
			body << YAML::Key << "RemoveRequirement";
			body << YAML::BeginMap;
			if (it_copyable->second.req_opt & 0x1)
				body << YAML::Key << "HpCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x4)
				body << YAML::Key << "SpCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x8)
				body << YAML::Key << "HpRateCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x10)
				body << YAML::Key << "SpRateCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x2)
				body << YAML::Key << "MaxHpTrigger" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x20)
				body << YAML::Key << "ZenyCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x40)
				body << YAML::Key << "Weapon" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x80)
				body << YAML::Key << "Ammo" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x100)
				body << YAML::Key << "State" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x200)
				body << YAML::Key << "Status" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x400)
				body << YAML::Key << "SpiritSphereCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x800)
				body << YAML::Key << "ItemCost" << YAML::Value << "true";
			if (it_copyable->second.req_opt & 0x1000)
				body << YAML::Key << "Equipment" << YAML::Value << "true";
			body << YAML::EndMap;
		}

		body << YAML::EndMap;
	}

	auto it_nearnpc = skill_nearnpc.find(skill_id);

	if (it_nearnpc != skill_nearnpc.end()) {
		body << YAML::Key << "NoNearNPC";
		body << YAML::BeginMap;

		if (it_nearnpc->second.unit_nonearnpc_range > 0)
			body << YAML::Key << "AdditionalRange" << YAML::Value << it_nearnpc->second.unit_nonearnpc_range;
		if (it_nearnpc->second.unit_nonearnpc_type > 0) {
			body << YAML::Key << "Type";
			body << YAML::BeginMap;
			if (it_nearnpc->second.unit_nonearnpc_type & 1)
				body << YAML::Key << "WarpPortal" << YAML::Value << "true";
			if (it_nearnpc->second.unit_nonearnpc_type & 2)
				body << YAML::Key << "Shop" << YAML::Value << "true";
			if (it_nearnpc->second.unit_nonearnpc_type & 4)
				body << YAML::Key << "Npc" << YAML::Value << "true";
			if (it_nearnpc->second.unit_nonearnpc_type & 8)
				body << YAML::Key << "Tomb" << YAML::Value << "true";
			body << YAML::EndMap;
		}

		body << YAML::EndMap;
	}

	if (strcmpi(split[9], "yes") != 0)
		body << YAML::Key << "CastCancel" << YAML::Value << "false";
	if (atoi(split[10]) != 0)
		body << YAML::Key << "CastDefenseReduction" << YAML::Value << atoi(split[10]);

	auto it_cast = skill_cast.find(skill_id);

	if (it_cast != skill_cast.end()) {
		if (!isMultiLevel(it_cast->second.cast)) {
			if (it_cast->second.cast[0] > 0)
				body << YAML::Key << "CastTime" << YAML::Value << it_cast->second.cast[0];
		} else {
			body << YAML::Key << "CastTime";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.cast); i++) {
				if (it_cast->second.cast[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.cast[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_cast->second.delay)) {
			if (it_cast->second.delay[0] > 0)
				body << YAML::Key << "AfterCastActDelay" << YAML::Value << it_cast->second.delay[0];
		} else {
			body << YAML::Key << "AfterCastActDelay";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.delay); i++) {
				if (it_cast->second.delay[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.delay[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_cast->second.walkdelay)) {
			if (it_cast->second.walkdelay[0] > 0)
				body << YAML::Key << "AfterCastWalkDelay" << YAML::Value << it_cast->second.walkdelay[0];
		} else {
			body << YAML::Key << "AfterCastWalkDelay";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.walkdelay); i++) {
				if (it_cast->second.walkdelay[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.walkdelay[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_cast->second.upkeep_time)) {
			if (it_cast->second.upkeep_time[0] != 0)
				body << YAML::Key << "Duration1" << YAML::Value << it_cast->second.upkeep_time[0];
		} else {
			body << YAML::Key << "Duration1";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.upkeep_time); i++) {
				if (it_cast->second.upkeep_time[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.upkeep_time[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (!isMultiLevel(it_cast->second.upkeep_time2)) {
			if (it_cast->second.upkeep_time2[0] != 0)
				body << YAML::Key << "Duration2" << YAML::Value << it_cast->second.upkeep_time2[0];
		} else {
			body << YAML::Key << "Duration2";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.upkeep_time2); i++) {
				if (it_cast->second.upkeep_time2[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.upkeep_time2[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_cast->second.cooldown)) {
			if (it_cast->second.cooldown[0] > 0)
				body << YAML::Key << "Cooldown" << YAML::Value << it_cast->second.cooldown[0];
		} else {
			body << YAML::Key << "Cooldown";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.cooldown); i++) {
				if (it_cast->second.cooldown[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.cooldown[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

#ifdef RENEWAL_CAST
		if (!isMultiLevel(it_cast->second.fixed_cast)) {
			if (it_cast->second.fixed_cast[0] != 0)
				body << YAML::Key << "FixedCastTime" << YAML::Value << it_cast->second.fixed_cast[0];
		} else {
			body << YAML::Key << "FixedCastTime";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_cast->second.fixed_cast); i++) {
				if (it_cast->second.fixed_cast[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Time" << YAML::Value << it_cast->second.fixed_cast[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
#endif
	}

	auto it_castdex = skill_castnodex.find(skill_id);

	if (it_castdex != skill_castnodex.end()) {
		if (it_castdex->second.castnodex > 0) {
			body << YAML::Key << "CastTimeFlags";
			body << YAML::BeginMap;

			if (it_castdex->second.castnodex & 1)
				body << YAML::Key << "IgnoreDex" << YAML::Value << "true";
			if (it_castdex->second.castnodex & 2)
				body << YAML::Key << "IgnoreStatus" << YAML::Value << "true";
			if (it_castdex->second.castnodex & 4)
				body << YAML::Key << "IgnoreItemBonus" << YAML::Value << "true";

			body << YAML::EndMap;
		}

		if (it_castdex->second.delaynodex > 0) {
			body << YAML::Key << "CastDelayFlags";
			body << YAML::BeginMap;

			if (it_castdex->second.delaynodex & 1)
				body << YAML::Key << "IgnoreDex" << YAML::Value << "true";
			if (it_castdex->second.delaynodex & 2)
				body << YAML::Key << "IgnoreStatus" << YAML::Value << "true";
			if (it_castdex->second.delaynodex & 4)
				body << YAML::Key << "IgnoreItemBonus" << YAML::Value << "true";

			body << YAML::EndMap;
		}
	}

	auto it_req = skill_require.find(skill_id);

	if (it_req != skill_require.end()) {
		body << YAML::Key << "Requires";
		body << YAML::BeginMap;
		
		if (!isMultiLevel(it_req->second.hp)) {
			if (it_req->second.hp[0] > 0)
				body << YAML::Key << "HpCost" << YAML::Value << it_req->second.hp[0];
		} else {
			body << YAML::Key << "HpCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.hp); i++) {
				if (it_req->second.hp[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.hp[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.sp)) {
			if (it_req->second.sp[0] > 0)
				body << YAML::Key << "SpCost" << YAML::Value << it_req->second.sp[0];
		} else {
			body << YAML::Key << "SpCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.sp); i++) {
				if (it_req->second.sp[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.sp[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.hp_rate)) {
			if (it_req->second.hp_rate[0] != 0)
				body << YAML::Key << "HpRateCost" << YAML::Value << it_req->second.hp_rate[0];
		} else {
			body << YAML::Key << "HpRateCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.hp_rate); i++) {
				if (it_req->second.hp_rate[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.hp_rate[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.sp_rate)) {
			if (it_req->second.sp_rate[0] != 0)
				body << YAML::Key << "SpRateCost" << YAML::Value << it_req->second.sp_rate[0];
		} else {
			body << YAML::Key << "SpRateCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.sp_rate); i++) {
				if (it_req->second.sp_rate[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.sp_rate[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.mhp)) {
			if (it_req->second.mhp[0] > 0)
				body << YAML::Key << "MaxHpTrigger" << YAML::Value << it_req->second.mhp[0];
		} else {
			body << YAML::Key << "MaxHpTrigger";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.mhp); i++) {
				if (it_req->second.mhp[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.mhp[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_req->second.zeny)) {
			if (it_req->second.zeny[0] > 0)
				body << YAML::Key << "ZenyCost" << YAML::Value << it_req->second.zeny[0];
		} else {
			body << YAML::Key << "ZenyCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.zeny); i++) {
				if (it_req->second.zeny[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.zeny[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (it_req->second.weapon != 0) {
			body << YAML::Key << "Weapon";
			body << YAML::BeginMap;

			int32 temp = it_req->second.weapon;

			if (temp != 99) { // Not "All"
				for (int32 i = 0; i < MAX_WEAPON_TYPE_ALL; i++) {
					if (i == MAX_WEAPON_TYPE)
						continue;

					if (temp & 1 << i) {
						constant = constant_lookup(i, "W_");
						constant.erase(0, 2);
						body << YAML::Key << name2Upper(constant) << YAML::Value << "true";
						temp ^= 1 << i;
					}
				}
			}

			body << YAML::EndMap;
		}

		if (it_req->second.ammo != 0) {
			body << YAML::Key << "Ammo";
			body << YAML::BeginMap;

			int32 temp = it_req->second.ammo;

			for (int32 i = 1; i < MAX_AMMO_TYPE; i++) {
				if (temp & 1 << i) {
					constant = constant_lookup(i, "AMMO_");
					constant.erase(0, 5);
					body << YAML::Key << name2Upper(constant) << YAML::Value << "true";
					temp ^= 1 << i;
				}
			}

			body << YAML::EndMap;
		}
		if (!isMultiLevel(it_req->second.ammo_qty)) {
			if (it_req->second.ammo_qty[0] > 0)
				body << YAML::Key << "AmmoAmount" << YAML::Value << it_req->second.ammo_qty[0];
		} else {
			body << YAML::Key << "AmmoAmount";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.ammo_qty); i++) {
				if (it_req->second.ammo_qty[i] > 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.ammo_qty[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (it_req->second.state) {
			constant = constant_lookup(it_req->second.state, "ST_");
			constant.erase(0, 3);
			body << YAML::Key << "State" << YAML::Value << name2Upper(constant);
		}

		if (it_req->second.status.size() > 0) {
			body << YAML::Key << "Status";
			body << YAML::BeginMap;

			for (const auto &it : it_req->second.status) {
				constant = constant_lookup(it, "SC_");
				constant.erase(0, 3);
				body << YAML::Key << name2Upper(constant) << YAML::Value << "true";
			}

			body << YAML::EndMap;
		}
		
		if (!isMultiLevel(it_req->second.spiritball)) {
			if (it_req->second.spiritball[0] != 0)
				body << YAML::Key << "SpiritSphereCost" << YAML::Value << it_req->second.spiritball[0];
		} else {
			body << YAML::Key << "SpiritSphereCost";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_req->second.spiritball); i++) {
				if (it_req->second.spiritball[i] != 0) {
					body << YAML::BeginMap;
					body << YAML::Key << "Level" << YAML::Value << i + 1;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.spiritball[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (it_req->second.itemid[0] > 0) {
			body << YAML::Key << "ItemCost";
			body << YAML::BeginSeq;

			for (uint8 i = 0; i < ARRAYLENGTH(it_req->second.itemid); i++) {
				if (it_req->second.itemid[i] > 0) {
					body << YAML::BeginMap;

					std::string *item_name = util::umap_find(aegis_itemnames, it_req->second.itemid[i]);

					if (item_name == nullptr) {
						ShowError("Item name for item id %hu is not known (itemcost).\n", it_req->second.itemid[i]);
						return false;
					}

					body << YAML::Key << "Item" << YAML::Value << *item_name;
					body << YAML::Key << "Amount" << YAML::Value << it_req->second.amount[i];

					switch (skill_id) { // List of level dependent item costs
						case WZ_FIREPILLAR:
							if (i < 6)
								break; // Levels 1-5 have no cost
						case NC_SHAPESHIFT:
						case NC_REPAIR:
							if (skill_id == NC_SHAPESHIFT || (skill_id == NC_REPAIR && i >= 5))
								break; // Don't add level 5 label as it exceeds the max level of these skills
						case GN_FIRE_EXPANSION:
						case SO_SUMMON_AGNI:
						case SO_SUMMON_AQUA:
						case SO_SUMMON_VENTUS:
						case SO_SUMMON_TERA:
						case SO_WATER_INSIGNIA:
						case SO_FIRE_INSIGNIA:
						case SO_WIND_INSIGNIA:
						case SO_EARTH_INSIGNIA:
						case KO_MAKIBISHI:
							body << YAML::Key << "Level" << YAML::Value << i;
					}

					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
		}

		if (it_req->second.eqItem.size() > 0) {
			body << YAML::Key << "Equipment";
			body << YAML::BeginMap;

			for (const auto &it : it_req->second.eqItem) {
				std::string *item_name = util::umap_find(aegis_itemnames, it);

				if (item_name == nullptr) {
					ShowError("Item name for item id %u is not known (equipment).\n", it);
					return false;
				}

				body << YAML::Key << *item_name << YAML::Value << "true";
			}

			body << YAML::EndMap;
		}

		body << YAML::EndMap;
	}

	auto it_unit = skill_unit.find(skill_id);

	if (it_unit != skill_unit.end()) {
		body << YAML::Key << "Unit";
		body << YAML::BeginMap;

		constant = constant_lookup(it_unit->second.unit_id, "UNT_");
		constant.erase(0, 4);
		body << YAML::Key << "Id" << YAML::Value << name2Upper(constant);
		if (it_unit->second.unit_id2 > 0) {
			constant = constant_lookup(it_unit->second.unit_id2, "UNT_");
			constant.erase(0, 4);
			body << YAML::Key << "AlternateId" << YAML::Value << name2Upper(constant);
		}
		
		if (!isMultiLevel(it_unit->second.unit_layout_type)) {
			if (it_unit->second.unit_layout_type[0] != 0)
				body << YAML::Key << "Layout" << YAML::Value << it_unit->second.unit_layout_type[0];
		} else {
			body << YAML::Key << "Layout";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_unit->second.unit_layout_type); i++) {
				if (it_unit->second.unit_layout_type[i] == 0 && i + 1 > 5)
					continue;
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Size" << YAML::Value << it_unit->second.unit_layout_type[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}
		
		if (!isMultiLevel(it_unit->second.unit_range)) {
			if (it_unit->second.unit_range[0] != 0)
				body << YAML::Key << "Range" << YAML::Value << it_unit->second.unit_range[0];
		} else {
			body << YAML::Key << "Range";
			body << YAML::BeginSeq;

			for (size_t i = 0; i < ARRAYLENGTH(it_unit->second.unit_range); i++) {
				if (it_unit->second.unit_range[i] == 0 && i + 1 > 5)
					continue;
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i + 1;
				body << YAML::Key << "Size" << YAML::Value << it_unit->second.unit_range[i];
				body << YAML::EndMap;
			}

			body << YAML::EndSeq;
		}

		if (it_unit->second.unit_interval != 0)
			body << YAML::Key << "Interval" << YAML::Value << it_unit->second.unit_interval;

		if (it_unit->second.target_str.size() > 0) {
			if (it_unit->second.target_str.compare("noenemy") == 0 || it_unit->second.target_str.compare("friend") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Friend";
			//else if (it_unit->second.target_str.compare("noenemy") == 0) // Same as Friend
			//	body << YAML::Key << "Target" << YAML::Value << "NoEnemy";
			else if (it_unit->second.target_str.compare("party") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Party";
			else if (it_unit->second.target_str.compare("ally") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Ally";
			else if (it_unit->second.target_str.compare("guild") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Guild";
			//else if (it_unit->second.target_str.compare("all") == 0)
			//	body << YAML::Key << "Target" << YAML::Value << "All";
			else if (it_unit->second.target_str.compare("enemy") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Enemy";
			else if (it_unit->second.target_str.compare("self") == 0)
				body << YAML::Key << "Target" << YAML::Value << "Self";
			else if (it_unit->second.target_str.compare("sameguild") == 0)
				body << YAML::Key << "Target" << YAML::Value << "SameGuild";
		}

		if (it_unit->second.unit_flag_csv > 0) {
			body << YAML::Key << "Flag";
			body << YAML::BeginMap;

			if (it_unit->second.unit_flag_csv & 0x1)
				body << YAML::Value << "NoEnemy" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x2)
				body << YAML::Value << "NoReiteration" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x4)
				body << YAML::Value << "NoFootSet" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x8)
				body << YAML::Value << "NoOverlap" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x10)
				body << YAML::Value << "PathCheck" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x20)
				body << YAML::Value << "NoPc" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x40)
				body << YAML::Value << "NoMob" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x80)
				body << YAML::Value << "Skill" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x100)
				body << YAML::Value << "Dance" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x200)
				body << YAML::Value << "Ensemble" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x400)
				body << YAML::Value << "Song" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x800)
				body << YAML::Value << "DualMode" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x1000)
				body << YAML::Value << "NoKnockback" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x2000)
				body << YAML::Value << "RangedSingleUnit" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x4000)
				body << YAML::Value << "CrazyWeedImmune" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x8000)
				body << YAML::Value << "RemovedByFireRain" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x10000)
				body << YAML::Value << "KnockbackGroup" << YAML::Value << "true";
			if (it_unit->second.unit_flag_csv & 0x20000)
				body << YAML::Value << "HiddenTrap" << YAML::Value << "true";

			body << YAML::EndMap;
		}

		body << YAML::EndMap;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from quest.cpp
static bool quest_read_db( char *split[], size_t columns, size_t current ){
	int32 quest_id = atoi(split[0]);

	if (quest_id < 0 || quest_id >= INT_MAX) {
		ShowError("quest_read_db: Invalid quest ID '%d'.\n", quest_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << quest_id;

	std::string title = split[17];
	
	if (columns > 18) { // If the title has a comma in it, concatenate
		size_t col = 18;

		while (col < columns) {
			title += ',' + std::string(split[col]);
			col++;
		}
	}

	title.erase(std::remove(title.begin(), title.end(), '"'), title.end()); // Strip double quotes out
	body << YAML::Key << "Title" << YAML::Value << title;

	if (strchr(split[1], ':') == nullptr) {
		uint32 time = atoi(split[1]);

		if (time > 0) {
			int32 day = time / 86400;

			time %= (24 * 3600);
			int32 hour = time / 3600;

			time %= 3600;
			int32 minute = time / 60;

			time %= 60;
			int32 second = time;

			std::string output = "+";

			if (day > 0)
				output += std::to_string(day) + "d";
			if (hour > 0)
				output += std::to_string(hour) + "h";
			if (minute > 0)
				output += std::to_string(minute) + "mn";
			if (second > 0)
				output += std::to_string(second) + "s";

			body << YAML::Key << "TimeLimit" << YAML::Value << output;
		}
	} else {
		if (*split[1]) {
			std::string time_str(split[1]), hour = time_str.substr(0, time_str.find(':')), output = {};

			time_str.erase(0, 3); // Remove "HH:"

			if (std::stoi(hour) > 0)
				output = std::to_string(std::stoi(hour)) + "h";
			if (std::stoi(time_str) > 0)
				output += std::to_string(std::stoi(time_str)) + "mn";

			body << YAML::Key << "TimeLimit" << YAML::Value << output; // No quests in TXT format had days, default to 0
		}
	}

	if (atoi(split[2]) > 0) {
		body << YAML::Key << "Targets";
		body << YAML::BeginSeq;

		for (size_t i = 0; i < MAX_QUEST_OBJECTIVES; i++) {
			int32 mob_id = (int32)atoi(split[i * 2 + 2]), count = atoi(split[i * 2 + 3]);

			if (!mob_id || !count)
				continue;

			std::string *mob_name = util::umap_find(aegis_mobnames, static_cast<uint16>(mob_id));

			if (!mob_name) {
				ShowError("quest_read_db: Invalid mob-class %hu, target not read.\n", mob_id);
				continue;
			}

			body << YAML::BeginMap;
			body << YAML::Key << "Mob" << YAML::Value << *mob_name;
			body << YAML::Key << "Count" << YAML::Value << count;
			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
	}

	if (atoi(split[2 * MAX_QUEST_OBJECTIVES + 2]) > 0) {
		body << YAML::Key << "Drops";
		body << YAML::BeginSeq;

		for (size_t i = 0; i < MAX_QUEST_DROPS; i++) {
			int32 mob_id = (int32)atoi(split[3 * i + (2 * MAX_QUEST_OBJECTIVES + 2)]);
			t_itemid nameid = strtoul(split[3 * i + (2 * MAX_QUEST_OBJECTIVES + 3)], nullptr, 10);

			if (!mob_id || !nameid)
				continue;

			std::string *mob_name = util::umap_find(aegis_mobnames, static_cast<uint16>(mob_id));

			if (!mob_name) {
				ShowError("quest_read_db: Invalid mob-class %hu, drop not read.\n", mob_id);
				continue;
			}

			std::string *item_name = util::umap_find(aegis_itemnames, nameid);

			if (!item_name) {
				ShowError("quest_read_db: Invalid item name %u, drop not read.\n", nameid);
				return false;
			}

			body << YAML::BeginMap;
			body << YAML::Key << "Mob" << YAML::Value << *mob_name;
			body << YAML::Key << "Item" << YAML::Value << *item_name;
			//body << YAML::Key << "Count" << YAML::Value << 1; // Default is 1
			body << YAML::Key << "Rate" << YAML::Value << atoi(split[3 * i + (2 * MAX_QUEST_OBJECTIVES + 4)]);
			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from instance.cpp
static bool instance_readdb_sub( char* str[], size_t columns, size_t current ){
	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << atoi(str[0]);
	body << YAML::Key << "Name" << YAML::Value << str[1];
	if (atoi(str[2]) != 3600)
		body << YAML::Key << "TimeLimit" << YAML::Value << atoi(str[2]);
	if (atoi(str[3]) != 300)
		body << YAML::Key << "IdleTimeOut" << YAML::Value << atoi(str[3]);
	body << YAML::Key << "Enter";
	body << YAML::BeginMap;
	body << YAML::Key << "Map" << YAML::Value << str[4];
	body << YAML::Key << "X" << YAML::Value << atoi(str[5]);
	body << YAML::Key << "Y" << YAML::Value << atoi(str[6]);
	body << YAML::EndMap;

	if (columns > 7) {
		body << YAML::Key << "AdditionalMaps";
		body << YAML::BeginMap;

		for( size_t i = 7; i < columns; i++ ){
			if (!strlen(str[i]))
				continue;

			if (strcmpi(str[4], str[i]) == 0)
				continue;

			body << YAML::Key << str[i] << YAML::Value << "true";
		}

		body << YAML::EndMap;
	}

	body << YAML::EndMap;

	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_itemavail( char *str[], size_t columns, size_t current ){
	item_avail.insert({ strtoul(str[0], nullptr, 10), strtoul(str[1], nullptr, 10) });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_buyingstore( char* fields[], size_t columns, size_t current ){
	item_buyingstore.insert({ strtoul(fields[0], nullptr, 10), true });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_flag( char* fields[], size_t columns, size_t current ){
	s_item_flag_csv2yaml item = { 0 };
	uint16 flag = abs(atoi(fields[1]));

	if (flag & 1)
		item.dead_branch = true;
	if (flag & 2)
		item.group = true;
	if (flag & 4)
		item.guid = true;
	if (flag & 8)
		item.bindOnEquip = true;
	if (flag & 16)
		item.broadcast = true;
	if (flag & 32)
		item.delay_consume = true;
	if (flag & 64)
		item.dropEffect = DROPEFFECT_CLIENT;
	else if (flag & 128)
		item.dropEffect = DROPEFFECT_WHITE_PILLAR;
	else if (flag & 256)
		item.dropEffect = DROPEFFECT_BLUE_PILLAR;
	else if (flag & 512)
		item.dropEffect = DROPEFFECT_YELLOW_PILLAR;
	else if (flag & 1024)
		item.dropEffect = DROPEFFECT_PURPLE_PILLAR;
	else if (flag & 2048)
		item.dropEffect = DROPEFFECT_ORANGE_PILLAR;

	item_flag.insert({ strtoul(fields[0], nullptr, 10), item });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_itemdelay( char* str[], size_t columns, size_t current ){
	s_item_delay_csv2yaml item = { 0 };

	item.delay = atoi(str[1]);

	if (columns == 3)
		item.sc = trim(str[2]);

	item_delay.insert({ strtoul(str[0], nullptr, 10), item });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_stack( char* fields[], size_t columns, size_t current ){
	s_item_stack_csv2yaml item = { 0 };

	item.amount = atoi(fields[1]);

	int32 type = strtoul(fields[2], nullptr, 10);

	if (type & 1)
		item.inventory = true;
	if (type & 2)
		item.cart = true;
	if (type & 4)
		item.storage = true;
	if (type & 8)
		item.guild_storage = true;

	item_stack.insert({ strtoul(fields[0], nullptr, 10), item });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_nouse( char* fields[], size_t columns, size_t current ){
	s_item_nouse_csv2yaml item = { 0 };

	item.sitting = "true";
	item.override = atoi(fields[2]);

	item_nouse.insert({ strtoul(fields[0], nullptr, 10), item });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_itemtrade( char* str[], size_t columns, size_t current ){
	s_item_trade_csv2yaml item = { 0 };
	int32 flag = atoi(str[1]);

	if (flag & 1)
		item.drop = true;
	if (flag & 2)
		item.trade = true;
	if (flag & 4)
		item.trade_partner = true;
	if (flag & 8)
		item.sell = true;
	if (flag & 16)
		item.cart = true;
	if (flag & 32)
		item.storage = true;
	if (flag & 64)
		item.guild_storage = true;
	if (flag & 128)
		item.mail = true;
	if (flag & 256)
		item.auction = true;

	item.override = atoi(str[2]);

	item_trade.insert({ strtoul(str[0], nullptr, 10), item });
	return true;
}

// Copied and adjusted from itemdb.cpp
static bool itemdb_read_db(const char* file) {
	FILE* fp = fopen(file, "r");

	if (fp == nullptr) {
		ShowError("can't read %s\n", file);
		return false;
	}

	int32 lines = 0;
	size_t entries = 0;
	char line[1024];

	while (fgets(line, sizeof(line), fp)) {
		char* str[32], * p;
		int32 i;

		lines++;

		if (line[0] == '/' && line[1] == '/')
			continue;

		memset(str, 0, sizeof(str));

		p = strstr(line, "//");

		if (p != nullptr) {
			*p = '\0';
		}

		p = line;
		while (ISSPACE(*p))
			++p;
		if (*p == '\0')
			continue;// empty line
		for (i = 0; i < 19; ++i) {
			str[i] = p;
			p = strchr(p, ',');
			if (p == nullptr)
				break;// comma not found
			*p = '\0';
			++p;
		}

		if (p == nullptr) {
			ShowError("itemdb_read_db: Insufficient columns in line %d (item with id %lu), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}

		// Script
		if (*p != '{') {
			ShowError("itemdb_read_db: Invalid format (Script column) in line %d (item with id %lu), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		str[19] = p + 1;
		p = strstr(p + 1, "},");
		if (p == nullptr) {
			ShowError("itemdb_read_db: Invalid format (Script column) in line %d (item with id %lu), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		*p = '\0';
		p += 2;

		// OnEquip_Script
		if (*p != '{') {
			ShowError("itemdb_read_db: Invalid format (OnEquip_Script column) in line %d (item with id %lu), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		str[20] = p + 1;
		p = strstr(p + 1, "},");
		if (p == nullptr) {
			ShowError("itemdb_read_db: Invalid format (OnEquip_Script column) in line %d (item with id %lu), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		*p = '\0';
		p += 2;

		// OnUnequip_Script (last column)
		if (*p != '{') {
			ShowError("itemdb_read_db: Invalid format (OnUnequip_Script column) in line %d (item with id %lu), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		str[21] = p;
		p = &str[21][strlen(str[21]) - 2];

		if (*p != '}') {
			/* lets count to ensure it's not something silly e.g. a extra space at line ending */
			int32 lcurly = 0, rcurly = 0;

			for (size_t v = 0; v < strlen(str[21]); v++) {
				if (str[21][v] == '{')
					lcurly++;
				else if (str[21][v] == '}') {
					rcurly++;
					p = &str[21][v];
				}
			}

			if (lcurly != rcurly) {
				ShowError("itemdb_read_db: Mismatching curly braces in line %d (item with id %lu), skipping.\n", lines, strtoul(str[0], nullptr, 10));
				continue;
			}
		}
		str[21] = str[21] + 1;  //skip the first left curly
		*p = '\0';              //null the last right curly

		t_itemid nameid = strtoul(str[0], nullptr, 10);

		body << YAML::BeginMap;
		body << YAML::Key << "Id" << YAML::Value << nameid;
		body << YAML::Key << "AegisName" << YAML::Value << str[1];
		body << YAML::Key << "Name" << YAML::Value << str[2];

		int32 type = atoi(str[3]), subtype = atoi(str[18]);

		const char* constant = constant_lookup( type, "IT_" );

		if( constant == nullptr ){
			ShowError( "itemdb_read_db: Unknown item type %d for item %u, skipping.\n", type, nameid );
			continue;
		}

		body << YAML::Key << "Type" << YAML::Value << name2Upper( constant + 3 );
		if( type == IT_WEAPON && subtype ){
			constant = constant_lookup( subtype, "W_" );

			if( constant == nullptr ){
				ShowError( "itemdb_read_db: Unknown weapon type %d for item %u, skipping.\n", subtype, nameid );
				continue;
			}

			body << YAML::Key << "SubType" << YAML::Value << name2Upper( constant + 2 );
		}else if( type == IT_AMMO && subtype ){
			constant = constant_lookup( subtype, "AMMO_" );

			if( constant == nullptr ){
				ShowError( "itemdb_read_db: Unknown ammo type %d for item %u, skipping.\n", subtype, nameid );
				continue;
			}

			body << YAML::Key << "SubType" << YAML::Value << name2Upper(constant + 5);
		}

		if (atoi(str[4]) > 0)
			body << YAML::Key << "Buy" << YAML::Value << atoi(str[4]);
		if (atoi(str[5]) > 0) {
			if (atoi(str[4]) / 2 != atoi(str[5]))
				body << YAML::Key << "Sell" << YAML::Value << atoi(str[5]);
		}
		if (atoi(str[6]) > 0)
			body << YAML::Key << "Weight" << YAML::Value << atoi(str[6]);

#ifdef RENEWAL
		int32 atk = 0, matk = 0;

		itemdb_re_split_atoi(str[7], &atk, &matk);
		if (atk > 0)
			body << YAML::Key << "Attack" << YAML::Value << atk;
		if (matk > 0)
			body << YAML::Key << "MagicAttack" << YAML::Value << matk;
#else
		if (atoi(str[7]) > 0)
			body << YAML::Key << "Attack" << YAML::Value << atoi(str[7]);
#endif
		if (atoi(str[8]) > 0)
			body << YAML::Key << "Defense" << YAML::Value << atoi(str[8]);
		if (atoi(str[9]) > 0)
			body << YAML::Key << "Range" << YAML::Value << atoi(str[9]);
		if (atoi(str[10]) > 0)
			body << YAML::Key << "Slots" << YAML::Value << atoi(str[10]);

		bool equippable = type == IT_UNKNOWN ? false : type == IT_ETC ? false : type == IT_CARD ? false : type == IT_PETEGG ? false : type == IT_PETARMOR ? false : type == IT_UNKNOWN2 ? false : true;

		if (equippable) {
			uint64 temp_mask = strtoull(str[11], nullptr, 0);

			if (temp_mask == 0) {
				//body << YAML::Key << "Jobs";
				//body << YAML::BeginMap << YAML::Key << "All" << YAML::Value << "false" << YAML::EndMap;
			} else if (temp_mask == 0xFFFFFFFF) { // Commented out because it's the default value
				//body << YAML::Key << "Jobs";
				//body << YAML::BeginMap << YAML::Key << "All" << YAML::Value << "true" << YAML::EndMap;
			} else if (temp_mask == 0xFFFFFFFE) {
				body << YAML::Key << "Jobs";
				body << YAML::BeginMap;
				body << YAML::Key << "All" << YAML::Value << "true";
				body << YAML::Key << "Novice" << YAML::Value << "false";
				body << YAML::Key << "SuperNovice" << YAML::Value << "false";
				body << YAML::EndMap;
			} else {
				body << YAML::Key << "Jobs";
				body << YAML::BeginMap;
				for (const auto &it : um_mapid2jobname) {
					uint64 job_mask = 1ULL << it.second;

					if ((temp_mask & job_mask) == job_mask)
						body << YAML::Key << it.first << YAML::Value << "true";
				}
				body << YAML::EndMap;
			}

			int32 temp_class = atoi(str[12]);

			if (temp_class == ITEMJ_NONE) {
				body << YAML::Key << "Classes";
				body << YAML::BeginMap << YAML::Key << "All" << YAML::Value << "false" << YAML::EndMap;
			} else if (temp_class == ITEMJ_ALL) { // Commented out because it's the default value
				//body << YAML::Key << "Classes";
				//body << YAML::BeginMap << YAML::Key << "All" << YAML::Value << "true" << YAML::EndMap;
			} else {
				body << YAML::Key << "Classes";
				body << YAML::BeginMap;
				if ((ITEMJ_THIRD & temp_class) && (ITEMJ_THIRD_UPPER & temp_class) && (ITEMJ_THIRD_BABY & temp_class)) {
					temp_class &= ~ITEMJ_ALL_THIRD;
					body << YAML::Key << "All_Third" << YAML::Value << "true";
				}
				if ((ITEMJ_UPPER & temp_class) && (ITEMJ_THIRD_UPPER & temp_class)) {
					temp_class &= ~ITEMJ_ALL_UPPER;
					body << YAML::Key << "All_Upper" << YAML::Value << "true";
				}
				if ((ITEMJ_BABY & temp_class) && (ITEMJ_THIRD_BABY & temp_class)) {
					temp_class &= ~ITEMJ_ALL_BABY;
					body << YAML::Key << "All_Baby" << YAML::Value << "true";
				}
				for (int32 i = ITEMJ_NONE; i <= ITEMJ_THIRD_BABY; i++) {
					if (i & temp_class) {
						const char* class_ = constant_lookup(i, "ITEMJ_");

						if (class_ != nullptr)
							body << YAML::Key << name2Upper(class_ + 6) << YAML::Value << "true";
					}
				}
				body << YAML::EndMap;
			}

			switch (atoi(str[13])) {
				case SEX_FEMALE:
					body << YAML::Key << "Gender" << YAML::Value << "Female";
					break;
				case SEX_MALE:
					body << YAML::Key << "Gender" << YAML::Value << "Male";
					break;
				//case SEX_BOTH: // Commented out because it's the default value
				//	body << YAML::Key << "Gender" << YAML::Value << "Both";
				//	break;
			}
		}
		if (atoi(str[14]) > 0) {
			int32 temp_loc = atoi(str[14]);

			body << YAML::Key << "Locations";
			body << YAML::BeginMap;
			if ((EQP_HAND_R & temp_loc) && (EQP_HAND_L & temp_loc)) {
				temp_loc &= ~EQP_ARMS;
				body << YAML::Key << "Both_Hand" << YAML::Value << "true";
			}
			if ((EQP_ACC_R & temp_loc) && (EQP_ACC_L & temp_loc)) {
				temp_loc &= ~EQP_ACC_RL;
				body << YAML::Key << "Both_Accessory" << YAML::Value << "true";
			}
			for (const auto &it : um_equipnames) {
				if (it.second & temp_loc)
					body << YAML::Key << it.first << YAML::Value << "true";
			}
			body << YAML::EndMap;
		}
		if (atoi(str[15]) > 0)
			body << YAML::Key << "WeaponLevel" << YAML::Value << atoi(str[15]);

		int32 elv = 0, elvmax = 0;

		itemdb_re_split_atoi(str[16], &elv, &elvmax);
		if (elv > 0)
			body << YAML::Key << "EquipLevelMin" << YAML::Value << elv;
		if (elvmax > 0)
			body << YAML::Key << "EquipLevelMax" << YAML::Value << elvmax;
		if (atoi(str[17]) > 0)
			body << YAML::Key << "Refineable" << YAML::Value << "true";
		if (strtoul(str[18], nullptr, 10) > 0 && type != IT_WEAPON && type != IT_AMMO)
			body << YAML::Key << "View" << YAML::Value << strtoul(str[18], nullptr, 10);

		auto it_avail = item_avail.find(nameid);

		if (it_avail != item_avail.end()) {
			std::string *item_name = util::umap_find(aegis_itemnames, static_cast<t_itemid>(it_avail->second));

			if (item_name == nullptr)
				ShowError("Item name for item id %u is not known (item_avail).\n", it_avail->second);
			else
				body << YAML::Key << "AliasName" << YAML::Value << *item_name;
		}

		auto it_flag = item_flag.find(nameid);
		auto it_buying = item_buyingstore.find(nameid);

		if (it_flag != item_flag.end() || it_buying != item_buyingstore.end()) {
			body << YAML::Key << "Flags";
			body << YAML::BeginMap;
			if (it_buying != item_buyingstore.end())
				body << YAML::Key << "BuyingStore" << YAML::Value << "true";
			if (it_flag != item_flag.end() && it_flag->second.dead_branch)
				body << YAML::Key << "DeadBranch" << YAML::Value << it_flag->second.dead_branch;
			if (it_flag != item_flag.end() && it_flag->second.group)
				body << YAML::Key << "Container" << YAML::Value << it_flag->second.group;
			if (it_flag != item_flag.end() && it_flag->second.guid)
				body << YAML::Key << "UniqueId" << YAML::Value << it_flag->second.guid;
			if (it_flag != item_flag.end() && it_flag->second.bindOnEquip)
				body << YAML::Key << "BindOnEquip" << YAML::Value << it_flag->second.bindOnEquip;
			if (it_flag != item_flag.end() && it_flag->second.broadcast)
				body << YAML::Key << "DropAnnounce" << YAML::Value << it_flag->second.broadcast;
			if (it_flag != item_flag.end() && it_flag->second.delay_consume)
				body << YAML::Key << "NoConsume" << YAML::Value << it_flag->second.delay_consume;
			if (it_flag != item_flag.end() && it_flag->second.dropEffect)
				body << YAML::Key << "DropEffect" << YAML::Value << name2Upper(constant_lookup(it_flag->second.dropEffect, "DROPEFFECT_") + 11);
			body << YAML::EndMap;
		}

		auto it_delay = item_delay.find(nameid);

		if (it_delay != item_delay.end()) {
			body << YAML::Key << "Delay";
			body << YAML::BeginMap;
			body << YAML::Key << "Duration" << YAML::Value << it_delay->second.delay;
			if (it_delay->second.sc.size() > 0)
				body << YAML::Key << "Status" << YAML::Value << name2Upper(it_delay->second.sc.erase(0, 3));
			body << YAML::EndMap;
		}

		auto it_stack = item_stack.find(nameid);

		if (it_stack != item_stack.end()) {
			body << YAML::Key << "Stack";
			body << YAML::BeginMap;
			body << YAML::Key << "Amount" << YAML::Value << it_stack->second.amount;
			if (it_stack->second.inventory)
				body << YAML::Key << "Inventory" << YAML::Value << it_stack->second.inventory;
			if (it_stack->second.cart)
				body << YAML::Key << "Cart" << YAML::Value << it_stack->second.cart;
			if (it_stack->second.storage)
				body << YAML::Key << "Storage" << YAML::Value << it_stack->second.storage;
			if (it_stack->second.guild_storage)
				body << YAML::Key << "GuildStorage" << YAML::Value << it_stack->second.guild_storage;
			body << YAML::EndMap;
		}

		auto it_nouse = item_nouse.find(nameid);

		if (it_nouse != item_nouse.end()) {
			body << YAML::Key << "NoUse";
			body << YAML::BeginMap;
			if (it_nouse->second.override != 100)
				body << YAML::Key << "Override" << YAML::Value << it_nouse->second.override;
			body << YAML::Key << "Sitting" << YAML::Value << "true";
			body << YAML::EndMap;
		}

		auto it_trade = item_trade.find(nameid);

		if (it_trade != item_trade.end()) {
			body << YAML::Key << "Trade";
			body << YAML::BeginMap;
			if (it_trade->second.override != 100)
				body << YAML::Key << "Override" << YAML::Value << it_trade->second.override;
			if (it_trade->second.drop)
				body << YAML::Key << "NoDrop" << YAML::Value << it_trade->second.drop;
			if (it_trade->second.trade)
				body << YAML::Key << "NoTrade" << YAML::Value << it_trade->second.trade;
			if (it_trade->second.trade_partner)
				body << YAML::Key << "TradePartner" << YAML::Value << it_trade->second.trade_partner;
			if (it_trade->second.sell)
				body << YAML::Key << "NoSell" << YAML::Value << it_trade->second.sell;
			if (it_trade->second.cart)
				body << YAML::Key << "NoCart" << YAML::Value << it_trade->second.cart;
			if (it_trade->second.storage)
				body << YAML::Key << "NoStorage" << YAML::Value << it_trade->second.storage;
			if (it_trade->second.guild_storage)
				body << YAML::Key << "NoGuildStorage" << YAML::Value << it_trade->second.guild_storage;
			if (it_trade->second.mail)
				body << YAML::Key << "NoMail" << YAML::Value << it_trade->second.mail;
			if (it_trade->second.auction)
				body << YAML::Key << "NoAuction" << YAML::Value << it_trade->second.auction;
			body << YAML::EndMap;
		}

		if (*str[19])
			body << YAML::Key << "Script" << YAML::Value << YAML::Literal << trim(str[19]);
		if (*str[20])
			body << YAML::Key << "EquipScript" << YAML::Value << YAML::Literal << trim(str[20]);
		if (*str[21])
			body << YAML::Key << "UnEquipScript" << YAML::Value << YAML::Literal << trim(str[21]);

		body << YAML::EndMap;
		entries++;
	}

	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' items in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file);

	return true;
}

// Copied and adjusted from itemdb.cpp
static bool itemdb_read_randomopt(const char* file) {
	FILE* fp = fopen( file, "r" );

	if( fp == nullptr ){
		ShowError( "Can't read %s\n", file );
		return 0;
	}

	uint32 lines = 0, count = 0;
	char line[1024];
	char path[256];

	while (fgets(line, sizeof(line), fp)) {
		char *str[2], *p;

		lines++;

		if (line[0] == '/' && line[1] == '/') // Ignore comments
			continue;

		memset(str, 0, sizeof(str));

		p = trim(line);

		if (*p == '\0')
			continue;// empty line

		if (!strchr(p, ','))
		{
			ShowError("itemdb_read_randomopt: Insufficient columns in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}

		str[0] = p;
		p = strchr(p, ',');
		*p = '\0';
		p++;

		str[1] = p;

		if (str[1][0] != '{') {
			ShowError("itemdb_read_randomopt(#1): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}

		/* no ending key anywhere (missing \}\) */
		if (str[1][strlen(str[1]) - 1] != '}') {
			ShowError("itemdb_read_randomopt(#2): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		} else {
			str[0] = trim(str[0]);

			int64 id = constant_lookup_int(str[0]);

			if (id == -100) {
				ShowError("itemdb_read_randomopt: Unknown random option '%s' constant, skipping.\n", str[0]);
				continue;
			}

			body << YAML::BeginMap;
			body << YAML::Key << "Id" << YAML::Value << id;
			body << YAML::Key << "Option" << YAML::Value << str[0] + 7;
			body << YAML::Key << "Script" << YAML::Literal << str[1];
			body << YAML::EndMap;

			rand_opt_db.insert({ count, str[0] + 7 });
		}
		count++;
	}

	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, file);

	return true;
}

// Copied and adjusted from itemdb.cpp
static bool itemdb_read_randomopt_group( char* str[], size_t columns, size_t current ){
	if ((columns - 2) % 3 != 0) {
		ShowError("itemdb_read_randomopt_group: Invalid column entries '%d'.\n", columns);
		return false;
	}

	uint16 id = static_cast<uint16>(rand_opt_group.size() + 1);
	s_random_opt_group_csv *group = util::umap_find(rand_opt_group, id);
	s_random_opt_group_csv group_entry;

	if (group == nullptr)
		group_entry.rate.push_back((uint16)strtoul(str[1], nullptr, 10));

	uint16 j = 0;
	for( size_t k = 2; k < columns && j < MAX_ITEM_RDM_OPT; k += 3 ){
		int32 randid_tmp = -1;

		for (const auto &opt : rand_opt_db) {
			if (opt.second.compare(str[k]) == 0) {
				randid_tmp = opt.first;
				break;
			}
		}

		if (randid_tmp < 0) {
			ShowError("itemdb_read_randomopt_group: Invalid random group id '%s' in column %d!\n", str[k], k + 1);
			continue;
		}

		std::vector<std::shared_ptr<s_random_opt_group_entry>> entries = {};

		if (group != nullptr)
			entries = group->slots[j];

		std::shared_ptr<s_random_opt_group_entry> entry = std::make_shared<s_random_opt_group_entry>();

		entry->id = static_cast<uint16>(randid_tmp);
		entry->min_value = (int16)strtoul(str[k + 1], nullptr, 10);
		entry->max_value = 0;
		entry->param = (int8)strtoul(str[k + 2], nullptr, 10);
		entry->chance = 0;
		entries.push_back(entry);
		if (group == nullptr)
			group_entry.slots[j] = entries;
		else
			group->slots[j] = entries;
		j++;
	}

	if (group == nullptr)
		rand_opt_group.insert({ id, group_entry });

	return true;
}

static bool itemdb_randomopt_group_yaml(void) {
	for (const auto &it : rand_opt_group) {
		body << YAML::BeginMap;
		body << YAML::Key << "Id" << YAML::Value << it.first;
		body << YAML::Key << "Group" << YAML::Value << it.second.name;
		body << YAML::Key << "Slots";
		body << YAML::BeginSeq;

		for (size_t i = 0; i < it.second.rate.size(); i++) {
			body << YAML::BeginMap;
			body << YAML::Key << "Slot" << YAML::Value << i + 1;

			body << YAML::Key << "Options";
			body << YAML::BeginSeq;

			for (size_t j = 0; j < it.second.slots.size(); j++) {
				std::vector<std::shared_ptr<s_random_opt_group_entry>> options = it.second.slots.at(static_cast<uint16>(j));

				for (const auto &opt_it : options) {
					body << YAML::BeginMap;

					for (const auto &opt : rand_opt_db) {
						if (opt.first == opt_it->id) {
							body << YAML::Key << "Option" << YAML::Value << opt.second;
							break;
						}
					}

					if (opt_it->min_value != 0)
						body << YAML::Key << "MinValue" << YAML::Value << opt_it->min_value;
					if (opt_it->param != 0)
						body << YAML::Key << "Param" << YAML::Value << opt_it->param;
					body << YAML::Key << "Chance" << YAML::Value << it.second.rate[i];
					body << YAML::EndMap;
				}
			}

			body << YAML::EndSeq;
			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
		body << YAML::EndMap;
	}

	return true;
}

static bool pc_readdb_levelpenalty( char* fields[], size_t columns, size_t current ){
	// 1=experience, 2=item drop
	int32 type = atoi( fields[0] );

	if( type != 1 && type != 2 ){
		ShowWarning( "pc_readdb_levelpenalty: Invalid type %d specified.\n", type );
		return false;
	}

	int64 val = constant_lookup_int( fields[1] );

	if( val == -100 ){
		ShowWarning("pc_readdb_levelpenalty: Unknown class constant %s specified.\n", fields[1] );
		return false;
	}

	int32 class_ = atoi( fields[1] );

	if( !CHK_CLASS( class_ ) ){
		ShowWarning( "pc_readdb_levelpenalty: Invalid class %d specified.\n", class_ );
		return false;
	}

	int32 diff = atoi( fields[2] );

	if( std::abs( diff ) > MAX_LEVEL ){
		ShowWarning( "pc_readdb_levelpenalty: Level difference %d is too high.\n", diff );
		return false;
	}

	diff += MAX_LEVEL - 1;

	level_penalty[type][class_][diff] = atoi(fields[3]);

	return true;
}

void pc_levelpenalty_yaml_sub( int32 type, const std::string& name ){
	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << name;
	body << YAML::Key << "LevelDifferences";
	body << YAML::BeginSeq;
	for( int32 i = ARRAYLENGTH( level_penalty[type][CLASS_NORMAL] ); i >= 0; i-- ){
		if( level_penalty[type][CLASS_NORMAL][i] > 0 && level_penalty[type][CLASS_NORMAL][i] != 100 ){
			body << YAML::BeginMap;
			body << YAML::Key << "Difference" << YAML::Value << ( i - MAX_LEVEL + 1 );
			body << YAML::Key << "Rate" << YAML::Value << level_penalty[type][CLASS_NORMAL][i];
			body << YAML::EndMap;
		}
	}
	body << YAML::EndSeq;
	body << YAML::EndMap;
}

bool pc_levelpenalty_yaml(){
	pc_levelpenalty_yaml_sub( 1, "Exp" );
	pc_levelpenalty_yaml_sub( 2, "Drop" );

	return true;
}


// mob_db.yml function
//--------------------
static bool mob_readdb_race2( char *fields[], size_t columns, size_t current ){
	int64 race;

	if (ISDIGIT(fields[0][0]))
		race = strtoll(fields[0], nullptr, 10);
	else if ((race = constant_lookup_int(fields[0])) == -100) {
		ShowWarning("mob_readdb_race2: Unknown race2 constant \"%s\".\n", fields[0]);
		return false;
	}

	std::vector<uint32> mobs;

	for (uint16 i = 1; i < columns; i++) {
		uint32 mob_id = strtol(fields[i], nullptr, 10);
		std::string *mob_name = util::umap_find(aegis_mobnames, static_cast<uint16>(mob_id));

		if (!mob_name) {
			ShowWarning("mob_readdb_race2: Unknown mob id %u for race2 %lld.\n", mob_id, race);
			continue;
		}

		mobs.push_back(mob_id);
	}

	mob_race2.insert({ static_cast<uint16>(race), mobs });

	return true;
}

// mob_db.yml function
//--------------------
static bool mob_readdb_drop( char *str[], size_t columns, size_t current ){
	uint32 mob_id = strtoul(str[0], nullptr, 10);
	std::string *mob_name = util::umap_find(aegis_mobnames, static_cast<uint16>(mob_id));

	if (!mob_name) {
		ShowWarning("mob_readdb_drop: Unknown mob ID %s.\n", str[0]);
		return false;
	}

	t_itemid nameid = strtoul(str[1], nullptr, 10);
	std::string *item_name = util::umap_find(aegis_itemnames, nameid);

	if (!item_name) {
		ShowWarning("mob_readdb_drop: Invalid item ID %s.\n", str[1]);
		return false;
	}

	int32 rate = atoi(str[2]);
	s_mob_drop_csv entry = {};
	uint8 flag = 0;

	if (columns > 4)
		flag = atoi(str[4]);

	entry.nameid = nameid;
	entry.rate = rate;
	entry.steal_protected = (flag & 1) ? 1 : 0;
	entry.mvp = (flag & 2) ? true : false;

	if (columns > 3)
		entry.group_string = trim(str[3]);

	const auto &exists = mob_drop.find(mob_id);

	if (exists != mob_drop.end())
		exists->second.push_back(entry);
	else {
		std::vector<s_mob_drop_csv> drop;

		drop.push_back(entry);
		mob_drop.insert({ mob_id, drop });
	}

	return true;
}

// Copied and adjusted from mob.cpp
static bool mob_readdb_sub( char *fields[], size_t columns, size_t current ){
	uint32 mob_id = strtoul(fields[0], nullptr, 10);

	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << mob_id;
	body << YAML::Key << "AegisName" << YAML::Value << fields[1];
	body << YAML::Key << "Name" << YAML::Value << fields[3];
	if (strcmp(fields[3], fields[2]) != 0)
		body << YAML::Key << "JapaneseName" << YAML::Value << fields[2];
	if (strtol(fields[4], nullptr, 10) > 0)
		body << YAML::Key << "Level" << YAML::Value << fields[4];
	if (strtol(fields[5], nullptr, 10) > 1)
		body << YAML::Key << "Hp" << YAML::Value << fields[5];
	if (strtol(fields[6], nullptr, 10) > 1)
		body << YAML::Key << "Sp" << YAML::Value << fields[6];
	if (strtol(fields[7], nullptr, 10) > 0)
		body << YAML::Key << "BaseExp" << YAML::Value << fields[7];
	if (strtol(fields[8], nullptr, 10) > 0)
		body << YAML::Key << "JobExp" << YAML::Value << fields[8];
	if (strtol(fields[30], nullptr, 10) > 0)
		body << YAML::Key << "MvpExp" << YAML::Value << fields[30];
	if (strtol(fields[10], nullptr, 10) > 0)
		body << YAML::Key << "Attack" << YAML::Value << fields[10];
	if (strtol(fields[11], nullptr, 10) > 0)
		body << YAML::Key << "Attack2" << YAML::Value << fields[11];
	if (strtol(fields[12], nullptr, 10) > 0)
		body << YAML::Key << "Defense" << YAML::Value << cap_value(std::stoi(fields[12]), DEFTYPE_MIN, DEFTYPE_MAX);
	if (strtol(fields[13], nullptr, 10) > 0)
		body << YAML::Key << "MagicDefense" << YAML::Value << cap_value(std::stoi(fields[13]), DEFTYPE_MIN, DEFTYPE_MAX);
	if (strtol(fields[14], nullptr, 10) != 1)
		body << YAML::Key << "Str" << YAML::Value << fields[14];
	if (strtol(fields[15], nullptr, 10) != 1)
		body << YAML::Key << "Agi" << YAML::Value << fields[15];
	if (strtol(fields[16], nullptr, 10) != 1)
		body << YAML::Key << "Vit" << YAML::Value << fields[16];
	if (strtol(fields[17], nullptr, 10) != 1)
		body << YAML::Key << "Int" << YAML::Value << fields[17];
	if (strtol(fields[18], nullptr, 10) != 1)
		body << YAML::Key << "Dex" << YAML::Value << fields[18];
	if (strtol(fields[19], nullptr, 10) != 1)
		body << YAML::Key << "Luk" << YAML::Value << fields[19];
	if (strtol(fields[9], nullptr, 10) > 0)
		body << YAML::Key << "AttackRange" << YAML::Value << fields[9];
	if (strtol(fields[20], nullptr, 10) > 0)
		body << YAML::Key << "SkillRange" << YAML::Value << fields[20];
	if (strtol(fields[21], nullptr, 10) > 0)
		body << YAML::Key << "ChaseRange" << YAML::Value << fields[21];
	if (fields[22])
		body << YAML::Key << "Size" << YAML::Value << constant_lookup(strtol(fields[22], nullptr, 10), "Size_") + 5;
	if (fields[23])
		body << YAML::Key << "Race" << YAML::Value << name2Upper(constant_lookup(strtol(fields[23], nullptr, 10), "RC_") + 3);

	std::string class_ = "Normal";
	bool header = false;

	for (const auto &race2 : mob_race2) {
		for (const auto &mobit : race2.second) {
			if (mobit == mob_id) {
				// These two races are now Class types
				if (race2.first == RC2_GUARDIAN) {
					class_ = "Guardian";
					continue;
				} else if (race2.first == RC2_BATTLEFIELD) {
					class_ = "Battlefield";
					continue;
				}

				if (!header) {
					body << YAML::Key << "RaceGroups";
					body << YAML::BeginMap;
					header = true;
				}

				body << YAML::Key << name2Upper(constant_lookup(race2.first, "RC2_") + 4) << YAML::Value << "true";

			}
		}
	}

	if (header)
		body << YAML::EndMap;

	if (fields[24]) {
		int32 ele = strtol(fields[24], nullptr, 10);

		body << YAML::Key << "Element" << YAML::Value << name2Upper(constant_lookup(ele % 20, "ELE_") + 4);
		body << YAML::Key << "ElementLevel" << YAML::Value << floor(ele / 20.);
	}
	if (strtol(fields[26], nullptr, 10) > 0)
		body << YAML::Key << "WalkSpeed" << YAML::Value << cap_value(std::stoi(fields[26]), MIN_WALK_SPEED, MAX_WALK_SPEED);
	if (strtol(fields[27], nullptr, 10) > 0)
		body << YAML::Key << "AttackDelay" << YAML::Value << fields[27];
	if (strtol(fields[28], nullptr, 10) > 0)
		body << YAML::Key << "AttackMotion" << YAML::Value << fields[28];
	if (strtol(fields[29], nullptr, 10) > 0)
		body << YAML::Key << "DamageMotion" << YAML::Value << fields[29];

	if (fields[25]) {
		uint32 mode = static_cast<e_mode>(strtoul(fields[25], nullptr, 0));
		std::string ai = "06";

		if ((mode & 0xC000000) == 0xC000000) {
			mode &= ~0xC000000;
			class_ = "Battlefield";
		} else if (class_.compare("Normal") == 0 && (mode & 0x6200000) == 0x6200000) { // Check for normal because class would have been changed above in RaceGroups
			mode &= ~0x6200000;
			class_ = "Boss";
		} else if ((mode & 0x4000000) == 0x4000000) {
			mode &= ~0x4000000;
			class_ = "Guardian";
		} else if ((mode & 0x1000000) == 0x1000000) {
			mode &= ~0x1000000;
			class_ = "Event";
		}

		if ((mode & MONSTER_TYPE_26) == MONSTER_TYPE_26) {
			mode &= ~MONSTER_TYPE_26;
			ai = "26";
		} else if ((mode & MONSTER_TYPE_27) == MONSTER_TYPE_27) {
			mode &= ~MONSTER_TYPE_27;
			ai = "27";
		} else if ((mode & MONSTER_TYPE_08) == MONSTER_TYPE_08) {
			mode &= ~MONSTER_TYPE_08;
			ai = "08";
		} else if ((mode & MONSTER_TYPE_04) == MONSTER_TYPE_04) {
			mode &= ~MONSTER_TYPE_04;
			ai = "04";
		} else if ((mode & MONSTER_TYPE_21) == MONSTER_TYPE_21) {
			mode &= ~MONSTER_TYPE_21;
			ai = "21";
		} else if ((mode & MONSTER_TYPE_20) == MONSTER_TYPE_20) {
			mode &= ~MONSTER_TYPE_20;
			ai = "20";
		} else if ((mode & MONSTER_TYPE_09) == MONSTER_TYPE_09) { // 9, 19
			mode &= ~MONSTER_TYPE_09;
			ai = "09";
		} else if ((mode & MONSTER_TYPE_13) == MONSTER_TYPE_13) {
			mode &= ~MONSTER_TYPE_13;
			ai = "13";
		} else if ((mode & MONSTER_TYPE_05) == MONSTER_TYPE_05) { // 5, 12
			mode &= ~MONSTER_TYPE_05;
			ai = "05";
		} else if ((mode & MONSTER_TYPE_07) == MONSTER_TYPE_07) {
			mode &= ~MONSTER_TYPE_07;
			ai = "07";
		} else if ((mode & MONSTER_TYPE_03) == MONSTER_TYPE_03) {
			mode &= ~MONSTER_TYPE_03;
			ai = "03";
		} else if ((mode & MONSTER_TYPE_24) == MONSTER_TYPE_24) {
			mode &= ~MONSTER_TYPE_24;
			ai = "24";
		} else if ((mode & MONSTER_TYPE_17) == MONSTER_TYPE_17) {
			mode &= ~MONSTER_TYPE_17;
			ai = "17";
		} else if ((mode & MONSTER_TYPE_10) == MONSTER_TYPE_10) { // 10, 11
			mode &= ~MONSTER_TYPE_10;
			ai = "10";
		} else if ((mode & MONSTER_TYPE_02) == MONSTER_TYPE_02) {
			mode &= ~MONSTER_TYPE_02;
			ai = "02";
		} else if ((mode & MONSTER_TYPE_01) == MONSTER_TYPE_01) {
			mode &= ~MONSTER_TYPE_01;
			ai = "01";
		} else if ((mode & MONSTER_TYPE_25) == MONSTER_TYPE_25) {
			mode &= ~MONSTER_TYPE_25;
			ai = "25";
		} else if ((mode & MONSTER_TYPE_06) == MONSTER_TYPE_06)
			ai = "06";

		if (ai.compare("06") != 0)
			body << YAML::Key << "Ai" << YAML::Value << ai;
		if (class_.compare("Normal") != 0)
			body << YAML::Key << "Class" << YAML::Value << class_;

		if (mode > 0) {
			body << YAML::Key << "Modes";
			body << YAML::BeginMap;
			if (mode & 0x1)
				body << YAML::Key << "CanMove" << YAML::Value << "true";
			if (mode & 0x80)
				body << YAML::Key << "CanAttack" << YAML::Value << "true";
			if (mode & 0x40)
				body << YAML::Key << "NoCast" << YAML::Value << "true";
			if (mode & 0x2)
				body << YAML::Key << "Looter" << YAML::Value << "true";
			if (mode & 0x4)
				body << YAML::Key << "Aggressive" << YAML::Value << "true";
			if (mode & 0x8)
				body << YAML::Key << "Assist" << YAML::Value << "true";
			if (mode & 0x20)
				body << YAML::Key << "NoRandomWalk" << YAML::Value << "true";
			if (mode & 0x200)
				body << YAML::Key << "CastSensorChase" << YAML::Value << "true";
			if (mode & 0x10)
				body << YAML::Key << "CastSensorIdle" << YAML::Value << "true";
			if (mode & 0x800)
				body << YAML::Key << "Angry" << YAML::Value << "true";
			if (mode & 0x400)
				body << YAML::Key << "ChangeChase" << YAML::Value << "true";
			if (mode & 0x1000)
				body << YAML::Key << "ChangeTargetMelee" << YAML::Value << "true";
			if (mode & 0x2000)
				body << YAML::Key << "ChangeTargetChase" << YAML::Value << "true";
			if (mode & 0x4000)
				body << YAML::Key << "TargetWeak" << YAML::Value << "true";
			if (mode & 0x8000)
				body << YAML::Key << "RandomTarget" << YAML::Value << "true";
			if (mode & 0x20000)
				body << YAML::Key << "IgnoreMagic" << YAML::Value << "true";
			if (mode & 0x10000)
				body << YAML::Key << "IgnoreMelee" << YAML::Value << "true";
			if (mode & 0x100000)
				body << YAML::Key << "IgnoreMisc" << YAML::Value << "true";
			if (mode & 0x40000)
				body << YAML::Key << "IgnoreRanged" << YAML::Value << "true";
			if (mode & 0x400000)
				body << YAML::Key << "TeleportBlock" << YAML::Value << "true";
			if (mode & 0x1000000)
				body << YAML::Key << "FixedItemDrop" << YAML::Value << "true";
			if (mode & 0x2000000)
				body << YAML::Key << "Detector" << YAML::Value << "true";
			if (mode & 0x200000)
				body << YAML::Key << "KnockBackImmune" << YAML::Value << "true";
			if (mode & 0x4000000)
				body << YAML::Key << "StatusImmune" << YAML::Value << "true";
			if (mode & 0x8000000)
				body << YAML::Key << "SkillImmune" << YAML::Value << "true";
			if (mode & 0x80000)
				body << YAML::Key << "Mvp" << YAML::Value << "true";
			body << YAML::EndMap;
		}
	}

	bool has_mvp_drops = false;

	for (uint8 d = 31; d < (31 + MAX_MVP_DROP * 2); d += 2) {
		if (strtoul(fields[d], nullptr, 10) > 0) {
			has_mvp_drops = true;
			break;
		}
	}

	if (has_mvp_drops) {
		body << YAML::Key << "MvpDrops";
		body << YAML::BeginSeq;

		for (uint8 i = 0; i < MAX_MVP_DROP; i++) {
			t_itemid nameid = strtoul(fields[31 + i * 2], nullptr, 10);

			if (nameid > 0) {
				std::string *item_name = util::umap_find(aegis_itemnames, nameid);

				if (!item_name) {
					ShowWarning("Monster \"%s\"(id: %u) is dropping an unknown item \"%u\"(MVP-Drop %d)\n", fields[1], mob_id, nameid, (i / 2) + 1);
					continue;
				}

				body << YAML::BeginMap;
				body << YAML::Key << "Item" << YAML::Value << *item_name;
				body << YAML::Key << "Rate" << YAML::Value << cap_value(std::stoi(fields[32 + i * 2]), 1, 10000);
				body << YAML::EndMap;
			}

			if (i < MAX_MVP_DROP - 1)
				continue;

			// Insert at end
			for (const auto &it : mob_drop) {
				if (it.first != mob_id)
					continue;

				for (const auto &drop : it.second) {
					if (drop.mvp) {
						std::string *item_name = util::umap_find(aegis_itemnames, drop.nameid);

						if (!item_name) {
							ShowWarning("Monster \"%s\"(id: %u) is dropping an unknown item \"%u\"(MVP-Drop %d)\n", fields[1], mob_id, drop.nameid, (i / 2) + 1);
							continue;
						}

						body << YAML::BeginMap;
						body << YAML::Key << "Item" << YAML::Value << *item_name;
						body << YAML::Key << "Rate" << YAML::Value << cap_value(drop.rate, 1, 10000);
						body << YAML::Key << "RandomOptionGroup" << YAML::Value << drop.group_string;
						body << YAML::EndMap;
					}
				}
			}
		}

		body << YAML::EndSeq;
	}

	bool has_drops = false;

	for (uint8 d = 31 + MAX_MVP_DROP * 2; d < ((31 + MAX_MVP_DROP * 2) + MAX_MOB_DROP * 2); d += 2) {
		if (strtoul(fields[d], nullptr, 10) > 0) {
			has_drops = true;
			break;
		}
	}

	if (has_drops) {
		body << YAML::Key << "Drops";
		body << YAML::BeginSeq;

		for (uint8 i = 0; i < MAX_MOB_DROP; i++) {
			int32 k = 31 + MAX_MVP_DROP * 2 + i * 2;
			t_itemid nameid = strtoul(fields[k], nullptr, 10);

			if (nameid > 0) {
				std::string *item_name = util::umap_find(aegis_itemnames, nameid);

				if (!item_name) {
					ShowWarning("Monster \"%s\"(id: %u) is dropping an unknown item \"%s\"\n", fields[1], mob_id, fields[k]);
					continue;
				}

				body << YAML::BeginMap;
				body << YAML::Key << "Item" << YAML::Value << *item_name;
				body << YAML::Key << "Rate" << YAML::Value << cap_value(std::stoi(fields[k + 1]), 1, 10000);
				if (i > 6) // Slots 8, 9, and 10 are inherently protected
					body << YAML::Key << "StealProtected" << YAML::Value << "true";
				body << YAML::EndMap;
			}

			if (i < MAX_MOB_DROP - 2 || i == MAX_MOB_DROP - 1)
				continue;

			// Insert before card
			for (const auto &it : mob_drop) {
				if (it.first != mob_id)
					continue;

				for (const auto &drop : it.second) {
					if (!drop.mvp) {
						std::string *item_name = util::umap_find(aegis_itemnames, drop.nameid);

						if (!item_name) {
							ShowWarning("Monster \"%s\"(id: %u) is dropping an unknown item \"%u\"\n", fields[1], mob_id, drop.nameid);
							continue;
						}

						body << YAML::BeginMap;
						body << YAML::Key << "Item" << YAML::Value << *item_name;
						body << YAML::Key << "Rate" << YAML::Value << cap_value(drop.rate, 1, 10000);
						body << YAML::Key << "RandomOptionGroup" << YAML::Value << drop.group_string;
						if (drop.steal_protected == 1)
							body << YAML::Key << "StealProtected" << YAML::Value << "true";
						body << YAML::EndMap;
					}
				}
			}
		}

		body << YAML::EndSeq;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from mob.cpp
static bool mob_parse_row_chatdb( char* fields[], size_t columns, size_t current ){
	int32 msg_id = atoi(fields[0]);

	if (msg_id <= 0){
		ShowError("Invalid chat ID '%d' in line %d\n", msg_id, current);
		return false;
	}

	char* msg = fields[2];
	size_t len = strlen(msg);

	while( len && ( msg[len-1] == '\r' || msg[len-1] == '\n' ) ) {// find EOL to strip
		len--;
	}

	if (len > (CHAT_SIZE_MAX-1)) {
		ShowError("Message too long! Line %d, id: %d\n", current, msg_id);
		return false;
	}
	else if (len == 0) {
		ShowWarning("Empty message for id %d.\n", msg_id);
		return false;
	}

	msg[len] = 0;  // strip previously found EOL

	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << msg_id;
	if (strcmp(fields[1], "0xFF0000") != 0)	// default color
		body << YAML::Key << "Color" << YAML::Value << fields[1];
	body << YAML::Key << "Dialog" << YAML::Value << msg;
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from homunculus.cpp
static bool read_homunculus_expdb(const char* file) {
	FILE* fp = fopen( file, "r" );

	if( fp == nullptr ){
		ShowError( "Can't read %s\n", file );
		return false;
	}

	uint32 lines = 0, count = 0;
	char line[1024];

	while (fgets(line, sizeof(line), fp)) {
		lines++;

		if (line[0] == '/' && line[1] == '/') // Ignore comments
			continue;

		t_exp exp = strtoull(line, nullptr, 10);

		if( exp > 0 ){
			body << YAML::BeginMap;
			body << YAML::Key << "Level" << YAML::Value << (count+1);
			body << YAML::Key << "Exp" << YAML::Value << exp;
			body << YAML::EndMap;
		}

		count++;
	}

	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, file);
	return true;
}

// Copied and adjusted from mob.cpp
static bool mob_readdb_group( char* str[], size_t columns, size_t current ){
	if (strncasecmp(str[0], "MOBG_", 5) != 0) {
		ShowError("The group %s must start with 'MOBG_'.\n", str[0]);
		return false;
	}

	uint16 mob_id = static_cast<uint16>(strtol(str[1], nullptr, 10));
	bool is_default = mob_id == 0;

	std::string group_name = str[0];
	group_name.erase(0, 5);
	std::transform(group_name.begin(), group_name.end(), group_name.begin(), ::toupper);

	s_randomsummon_group_csv2yaml *group = util::map_find(summon_group, group_name);
	bool exists = group != nullptr;

	s_randomsummon_group_csv2yaml group_entry;

	if (is_default)
		mob_id = static_cast<uint16>(strtol(str[3], nullptr, 10));
	
	std::string *mob_name = util::umap_find(aegis_mobnames, mob_id);

	if (!mob_name) {
		ShowError("Unknown mob id %d for group %s.\n", mob_id, str[0]);
		return false;
	}

	if (!exists) {
		group_entry.default_mob = *mob_name;
		group_entry.group_name = group_name;
	}

	if (is_default) {
		if (exists)
			group->default_mob = *mob_name;
		else
			summon_group.insert({ group_name, group_entry });
	}
	else {
		std::shared_ptr<s_randomsummon_entry_csv2yaml> entry = std::make_shared<s_randomsummon_entry_csv2yaml>();

		entry->mob_name = *mob_name;
		entry->rate = strtol(str[3], nullptr, 10);

		if (exists)
			group->list.push_back(entry);
		else {
			group_entry.list.push_back(entry);
			summon_group.insert({ group_name, group_entry });
		}
	}

	return true;
}

static bool mob_readdb_group_yaml(void) {
	for (const auto &it : summon_group) {
		body << YAML::BeginMap;
		body << YAML::Key << "Group" << YAML::Value << it.first;
		body << YAML::Key << "Default" << YAML::Value << it.second.default_mob;
		body << YAML::Key << "Summon";
		body << YAML::BeginSeq;
		for (const auto &sumit : it.second.list) {
			body << YAML::BeginMap;
			body << YAML::Key << "Mob" << YAML::Value << sumit->mob_name;
			body << YAML::Key << "Rate" << YAML::Value << sumit->rate;
			body << YAML::EndMap;
		}
		body << YAML::EndSeq;
		body << YAML::EndMap;
	}
	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_createarrowdb( char* split[], size_t columns, size_t current ){
	t_itemid nameid = static_cast<t_itemid>(strtoul(split[0], nullptr, 10));

	if (nameid == 0)
		return true;

	std::string *material_name = util::umap_find(aegis_itemnames, nameid);

	if (!material_name) {
		ShowError("skill_parse_row_createarrowdb: Invalid item %u.\n", nameid);
		return false;
	}

	// Import just for clearing/disabling from original data
	if (strtoul(split[1], nullptr, 10) == 0) {
		body << YAML::BeginMap;
		body << YAML::Key << "Remove" << YAML::Value << *material_name;
		body << YAML::EndMap;
		return true;
	}

	std::map<std::string, uint32> item_created;
	
	for( size_t x = 1; x + 1 < columns && split[x] && split[x + 1]; x += 2 ){
		nameid = static_cast<t_itemid>(strtoul(split[x], nullptr, 10));
		std::string* item_name = util::umap_find(aegis_itemnames, nameid);

		if (!item_name) {
			ShowError("skill_parse_row_createarrowdb: Invalid item %u.\n", nameid);
			return false;
		}

		item_created.insert({ *item_name, strtoul(split[x+1], nullptr, 10) });
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Source" << YAML::Value << *material_name;
	body << YAML::Key << "Make";
	body << YAML::BeginSeq;
	for (const auto &it : item_created) {
		body << YAML::BeginMap;
		body << YAML::Key << "Item" << YAML::Value << it.first;
		body << YAML::Key << "Amount" << YAML::Value << it.second;
		body << YAML::EndMap;
	}
	body << YAML::EndSeq;
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from pc.cpp
static bool pc_read_statsdb(const char* file) {
	FILE* fp = fopen( file, "r" );

	if( fp == nullptr ){
		ShowError( "Can't read %s\n", file );
		return false;
	}

	uint32 lines = 0, count = 0;
	char line[1024];
	
	while (fgets(line, sizeof(line), fp)) {
		lines++;

		trim(line);
		if (line[0] == '/' && line[1] == '/') // Ignore comments
			continue;

		body << YAML::BeginMap;
		body << YAML::Key << "Level" << YAML::Value << (count+1);
		body << YAML::Key << "Points" << YAML::Value << static_cast<uint32>(strtoul(line, nullptr, 10));
		body << YAML::EndMap;

		count++;
	}

	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, file);
	return true;
}

// Copied and adjusted from guild.cpp
static bool guild_read_castledb( char* str[], size_t columns, size_t current ){
	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << str[0];
	body << YAML::Key << "Map" << YAML::Value << str[1];
	body << YAML::Key << "Name" << YAML::Value << trim(str[2]);
	body << YAML::Key << "Npc" << YAML::Value << trim(str[3]);
	body << YAML::EndMap;
	return true;
}

// Copied and adjusted from int_guild.cpp
static bool exp_guild_parse_row( char* split[], size_t column, size_t current ){
	t_exp exp = strtoull(split[0], nullptr, 10);

	if (exp > MAX_GUILD_EXP) {
		ShowError("exp_guild: Invalid exp %" PRIu64 " at line %d, exceeds max of %" PRIu64 "\n", exp, current, MAX_GUILD_EXP);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Level" << YAML::Value << (current+1);
	body << YAML::Key << "Exp" << YAML::Value << exp;
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from itemdb.cpp
static bool itemdb_read_group( char* str[], size_t columns, size_t current ){
	if (strncasecmp(str[0], "IG_", 3) != 0) {
		ShowError("The group %s must start with 'IG_'.\n", str[0]);
		return false;
	}
	if (columns < 3) {
		ShowError("itemdb_read_group: Insufficient columns (found %d, need at least 3).\n", columns);
		return false;
	}

	std::string group_name = trim(str[0]);
	group_name.erase(0, 3);
	std::transform(group_name.begin(), group_name.end(), group_name.begin(), ::toupper);

	if (strcmpi( str[1], "clear" ) == 0) {
		item_group.erase(group_name);
		return true;
	}

	// check item name
	str[1] = trim(str[1]);
	t_itemid nameid = strtoul(str[1], nullptr, 10);

	std::string *name = util::umap_find(aegis_itemnames, nameid);
	std::string item_name;

	if (name)
		item_name = *name;
	else {
		std::string tmp = str[1];
		auto it = aegis_itemnames.begin();

		for (; it != aegis_itemnames.end(); ++it) {
			if (it->second == tmp)
				break;
		}
		if (it == std::end(aegis_itemnames)) {
			ShowWarning( "itemdb_read_group: Non-existant item '%s'\n", str[1] );
			return false;
		}
		item_name = it->second;
	}

	// Checking sub group
	uint32 prob = atoi(str[2]);
	uint32 rand_group;

	if (columns < 5)
		rand_group = 1;	// default
	else
		rand_group = atoi(str[4]);

	if (rand_group > 0 && prob == 0) {
		ShowWarning( "itemdb_read_group: Random item must have a probability. Group '%s'\n", str[0] );
		return false;
	}

	// update group
	s_item_group_db_csv2yaml *group = util::map_find(item_group, group_name);
	s_item_group_db_csv2yaml group_entry;

	bool exists = group != nullptr;

	if (!exists)
		group_entry.group_name = group_name;

	s_item_group_entry_csv2yaml entry = {};

	entry.item_name = item_name;
	if (columns > 3) {
		int32 amount = cap_value(atoi(str[3]),1,MAX_AMOUNT);
		if (amount > 1)
			entry.amount = amount;
	}
	if (columns > 5) entry.isAnnounced = atoi(str[5]) > 0;
	if (columns > 6) entry.duration = cap_value(atoi(str[6]),0,UINT16_MAX);
	if (columns > 7) entry.GUID = atoi(str[7]) > 0;
	if (columns > 8) {
		int32 bound = cap_value(atoi(str[8]),BOUND_NONE,BOUND_MAX-1);
		if (bound > 0) {
			std::string constant = constant_lookup(bound, "BOUND_");
			constant.erase(0, 6);
			entry.bound = constant;
		}
	}
	if (columns > 9) entry.isNamed = atoi(str[9]) > 0;

	// in this case, we add x2 entries to keep the previous system: x1 in subgroup 0 without rate (must) and x1 in subgroup 1 with rate (random)
	if (rand_group == 0 && prob > 0) {
		if (exists)
			group->item[rand_group].push_back(entry);
		else
			group_entry.item[rand_group].push_back(entry);
		rand_group = 1;
	}

	entry.rate = prob;

	if (exists)
		group->item[rand_group].push_back(entry);
	else {
		group_entry.item[rand_group].push_back(entry);
		item_group.insert({ group_name, group_entry });
	}

	return true;
}

static bool itemdb_read_group_yaml(void) {
	for (const auto &it : item_group) {
		body << YAML::BeginMap;
		body << YAML::Key << "Group" << YAML::Value << it.first;
		body << YAML::Key << "SubGroups";
		body << YAML::BeginSeq;

		for (const auto &item : it.second.item) {	// subgroup
			body << YAML::BeginMap;
			body << YAML::Key << "SubGroup" << YAML::Value << item.first;
			if (item.first == 0)
				body << YAML::Key << "Algorithm" << YAML::Value << "All";
			else if (item.first == 6)
				body << YAML::Key << "Algorithm" << YAML::Value << "Random";
			body << YAML::Key << "List";
			body << YAML::BeginSeq;
			for (const auto &listit : item.second) {
				body << YAML::BeginMap;
				body << YAML::Key << "Item" << YAML::Value << listit.item_name;
				if (listit.rate > 0)
					body << YAML::Key << "Rate" << YAML::Value << listit.rate;
				if (listit.amount > 1)
					body << YAML::Key << "Amount" << YAML::Value << listit.amount;
				if (listit.duration > 0)
					body << YAML::Key << "Duration" << YAML::Value << listit.duration;
				if (listit.isAnnounced)
					body << YAML::Key << "Announced" << YAML::Value << "true";
				if (listit.GUID)
					body << YAML::Key << "UniqueId" << YAML::Value << "true";
				if (listit.isNamed)
					body << YAML::Key << "Named" << YAML::Value << "true";
				if (!listit.bound.empty())
					body << YAML::Key << "Bound" << YAML::Value << listit.bound;
				body << YAML::EndMap;
			}
			body << YAML::EndSeq;
			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
		body << YAML::EndMap;
	}
	return true;
}

// Copied and adjusted from mob.cpp
static bool mob_readdb_itemratio( char* str[], size_t columns, size_t current ){
	t_itemid nameid = strtoul(str[0], nullptr, 10);

	std::string *item_name = util::umap_find(aegis_itemnames, nameid);

	if (!item_name) {
		ShowWarning("mob_readdb_itemratio: Invalid item id %u.\n", nameid);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Item" << YAML::Value << *item_name;
	body << YAML::Key << "Ratio" << YAML::Value << strtoul(str[1], nullptr, 10);

	if (columns-2 > 0) {
		body << YAML::Key << "List";
		body << YAML::BeginMap;
		for( size_t i = 0; i < columns - 2; i++ ){
			uint16 mob_id = static_cast<uint16>(strtoul(str[i+2], nullptr, 10));
			std::string* mob_name = util::umap_find( aegis_mobnames, mob_id );

			if (mob_name == nullptr) {
				ShowWarning( "mob_readdb_itemratio: Invalid monster with ID %hu.\n", mob_id );
				continue;
			}
			body << YAML::Key << *mob_name << YAML::Value << "true";
		}
		body << YAML::EndMap;
	}

	body << YAML::EndMap;
	return true;
}

// Copied and adjusted from status.cpp
static bool status_readdb_attrfix(const char* file) {
	FILE* fp = fopen( file, "r" );

	if( fp == nullptr ){
		ShowError( "Can't read %s\n", file );
		return false;
	}

	uint32 lines = 0, count = 0;
	char line[1024];
	int32 lv, i, j;
	std::string constant;

	while (fgets(line, sizeof(line), fp)) {
		lines++;

		if (line[0] == '/' && line[1] == '/')
			continue;

		lv = strtoul(line, nullptr, 10);
		if (!CHK_ELEMENT_LEVEL(lv))
			continue;

		body << YAML::BeginMap;
		body << YAML::Key << "Level" << YAML::Value << (count+1);

		for (i = 0; i < ELE_ALL; ) {
			char *p;
			if (!fgets(line, sizeof(line), fp))
				break;
			if (line[0] == '/' && line[1] == '/')
				continue;

			constant = constant_lookup(i, "Ele_");
			constant.erase(0, 4);
			body << YAML::Key << name2Upper(constant);
			body << YAML::BeginMap;

			for (j = 0, p = line; j < ELE_ALL && p; j++) {
				while (*p == 32) //skipping space (32=' ')
					p++;

				constant = constant_lookup(j, "Ele_");
				constant.erase(0, 4);
				body << YAML::Key << name2Upper(constant) << YAML::Value << atoi(p);
	
				p = strchr(p, ',');
				if (p)
					*p++=0;
			}
			body << YAML::EndMap;

			i++;
		}
		body << YAML::EndMap;

		count++;
	}

	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, file);
	return true;
}

// Copied and adjusted from script.cpp
static bool read_constdb( char* fields[], size_t columns, size_t current ){
	char name[1024], val[1024];
	int32 type = 0;

	if( columns > 1 ){
		if( sscanf(fields[0], "%1023[A-Za-z0-9/_]", name) != 1 ||
			sscanf(fields[1], "%1023[A-Za-z0-9/_]", val) != 1 || 
			( columns >= 2 && sscanf(fields[2], "%11d", &type) != 1 ) ){
			ShowWarning("Skipping line '" CL_WHITE "%d" CL_RESET "', invalid constant definition\n", current);
			return false;
		}
	}else{
		if( sscanf(fields[0], "%1023[A-Za-z0-9/_] %1023[A-Za-z0-9/_-] %11d", name, val, &type) < 2 ){
			ShowWarning( "Skipping line '" CL_WHITE "%d" CL_RESET "', invalid constant definition\n", current );
			return false;
		}
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Name" << YAML::Value << name;
	body << YAML::Key << "Value" << YAML::Value << val;
	if (type != 0)
		body << YAML::Key << "Parameter" << YAML::Value << "true";
	body << YAML::EndMap;

	return true;
}

// job_db.yml function
//----------------------
static bool pc_readdb_job2( char* fields[], size_t columns, size_t current ){
	std::vector<int32> stats;

	stats.resize(MAX_LEVEL);
	std::fill(stats.begin(), stats.end(), 0); // Fill with 0 so we don't produce arbitrary stats

	for( size_t i = 1; i < columns; i++ ){
		stats[i - 1] = atoi(fields[i]);
	}

	job_db2.insert({ atoi(fields[0]), stats });
	return true;
}

// job_db.yml function
//----------------------
static bool pc_readdb_job_param( char* fields[], size_t columns, size_t current ){
	int32 job_id = atoi(fields[0]);
	s_job_param entry = {};

	entry.str = atoi(fields[1]);
	entry.agi = atoi(fields[2]) ? atoi(fields[2]) : entry.str;
	entry.vit = atoi(fields[3]) ? atoi(fields[3]) : entry.str;
	entry.int_ = atoi(fields[4]) ? atoi(fields[4]) : entry.str;
	entry.dex = atoi(fields[5]) ? atoi(fields[5]) : entry.str;
	entry.luk = atoi(fields[6]) ? atoi(fields[6]) : entry.str;

	job_param.insert({ job_id, entry });

	return true;
}

// job_basehpsp_db.yml function
//----------------------
static bool pc_readdb_job_exp_sub( char* fields[], size_t columns, size_t current ){
	int32 level = atoi(fields[0]), jobs[CLASS_COUNT], job_count = skill_split_atoi(fields[1], jobs, CLASS_COUNT), type = atoi(fields[2]);

	for (int32 i = 0; i < job_count; i++) {
		if (type == 0)
			exp_base_level.insert({ jobs[i], level });
		else
			exp_job_level.insert({ jobs[i], level });
	}

	return true;
}

// Copied and adjusted from pc.cpp
static bool pc_readdb_job_exp( char* fields[], size_t columns, size_t current ){
	int32 level = atoi(fields[0]), jobs[CLASS_COUNT], job_count = skill_split_atoi(fields[1], jobs, CLASS_COUNT), type = atoi(fields[2]);

	body << YAML::BeginMap;
	body << YAML::Key << "Jobs";
	body << YAML::BeginMap;
	for (int32 i = 0; i < job_count; i++) {
		body << YAML::Key << name2Upper(constant_lookup(jobs[i], "JOB_") + 4) << YAML::Value << "true";
		if (type == 0)
			exp_base_level.insert({ jobs[i], level });
		else
			exp_job_level.insert({ jobs[i], level });
	}
	body << YAML::EndMap;

	if (type == 0) {
		body << YAML::Key << "MaxBaseLevel" << YAML::Value << level;
		body << YAML::Key << "BaseExp";
	} else {
		body << YAML::Key << "MaxJobLevel" << YAML::Value << level;
		body << YAML::Key << "JobExp";
	}
	body << YAML::BeginSeq;

	for (int32 i = 0; i < level; i++) {
		body << YAML::BeginMap;
		body << YAML::Key << "Level" << YAML::Value << i + 1;
		body << YAML::Key << "Exp" << YAML::Value << strtoll(fields[3 + i], nullptr, 10);
		body << YAML::EndMap;
	}

	body << YAML::EndSeq;
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from pc.cpp
static bool pc_readdb_job_basehpsp( char* fields[], size_t columns, size_t current ){
	int32 type = atoi(fields[3]), jobs[CLASS_COUNT], job_count = skill_split_atoi(fields[2], jobs, CLASS_COUNT);

	body << YAML::BeginMap;
	body << YAML::Key << "Jobs";
	body << YAML::BeginMap;
	for (int32 i = 0; i < job_count; i++)
		body << YAML::Key << name2Upper(constant_lookup(jobs[i], "JOB_") + 4) << YAML::Value << "true";
	body << YAML::EndMap;

	if (type == 0)
		body << YAML::Key << "BaseHp";
	else
		body << YAML::Key << "BaseSp";
	body << YAML::BeginSeq;

	int32 j = 0, job_id = jobs[0], endlvl = 0;

	// Find the highest level in the group of jobs
	for (int32 i = 0; i < job_count; i++) {
		auto it_level = exp_base_level.find(jobs[i]);
		int32 tmplvl;

		if (it_level != exp_base_level.end())
			tmplvl = it_level->second;
		else {
			ShowError("pc_readdb_job_basehpsp: The job_exp database needs to be imported into memory before converting the job_basehpsp_db database.\n");
			return false;
		}

		if (endlvl < tmplvl)
			endlvl = tmplvl;
	}

	// These jobs don't have values less than level 99
	if ((job_id >= JOB_RUNE_KNIGHT && job_id <= JOB_BABY_MECHANIC2) || job_id == JOB_KAGEROU || job_id == JOB_OBORO || job_id == JOB_REBELLION || job_id == JOB_BABY_KAGEROU || job_id == JOB_BABY_OBORO || job_id == JOB_BABY_REBELLION || job_id >= JOB_STAR_EMPEROR)
		j = 98;

	if (type == 0) { // HP
		for (; j < endlvl; j++) {
			if (atoi(fields[j + 4])) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << j + 1;
				body << YAML::Key << "Hp" << YAML::Value << strtoll(fields[j + 4], nullptr, 10);
				body << YAML::EndMap;
			}
		}
	} else { // SP
		for (; j < endlvl; j++) {
			if (atoi(fields[j + 4])) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << j + 1;
				body << YAML::Key << "Sp" << YAML::Value << strtoll(fields[j + 4], nullptr, 10);
				body << YAML::EndMap;
			}
		}
	}
	body << YAML::EndSeq;
	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from pc.cpp
static bool pc_readdb_job1( char* fields[], size_t columns, size_t current ){
	int32 job_id = atoi(fields[0]);

	if (job_id == JOB_WEDDING)
		return true;

	body << YAML::BeginMap;
	body << YAML::Key << "Jobs";
	body << YAML::BeginMap;
	body << YAML::Key << name2Upper(constant_lookup(job_id, "JOB_") + 4) << YAML::Value << "true";
	body << YAML::EndMap;
	if (atoi(fields[1]) != 20000)
		body << YAML::Key << "MaxWeight" << YAML::Value << atoi(fields[1]);
	if (atoi(fields[2]) != 0)
		body << YAML::Key << "HpFactor" << YAML::Value << atoi(fields[2]);
	if (atoi(fields[3]) != 500)
		body << YAML::Key << "HpIncrease" << YAML::Value << atoi(fields[3]);
	if (atoi(fields[4]) != 100)
		body << YAML::Key << "SpIncrease" << YAML::Value << atoi(fields[4]);

	body << YAML::Key << "BaseASPD";
	body << YAML::BeginMap;

#ifdef RENEWAL_ASPD
	for (int32 i = 0; i <= MAX_WEAPON_TYPE; i++) {
		if (atoi(fields[i + 5]) != 200) {
#else
	for (int32 i = 0, j = 0; i < MAX_WEAPON_TYPE; i++) {
		if (atoi(fields[i + 5]) != 2000) {
#endif
			const char *weapon = constant_lookup(i, "W_");

			if (weapon == nullptr) {
				ShowError("pc_readdb_job1: Invalid weapon type found, skipping.\n");
				continue;
			}

			body << YAML::Key << name2Upper(weapon + 2) << YAML::Value << atoi(fields[i + 5]);
		}
	}

	body << YAML::EndMap;

	auto job_bonus = job_db2.find(job_id);
	auto jlvl = exp_job_level.find(job_id);

	if (job_bonus != job_db2.end() && job_id != JOB_BABY) {
		body << YAML::Key << "BonusStats";
		body << YAML::BeginSeq;

		for (int32 i = 1; i <= jlvl->second; i++) {
			auto value = job_bonus->second[i - 1];

			if( value == 0 ){
				continue;
			}

			const char *bonus = constant_lookup( value - 1, "PARAM_" );

			if (bonus != nullptr) {
				body << YAML::BeginMap;
				body << YAML::Key << "Level" << YAML::Value << i;
				body << YAML::Key << name2Upper(bonus + 6) << YAML::Value << "1";
				body << YAML::EndMap;
			}
		}

		body << YAML::EndSeq;
	}

	auto param = job_param.find(job_id);

	if (param != job_param.end()) {
		body << YAML::Key << "MaxStats";
		body << YAML::BeginMap;

		if (param->second.str > 0)
			body << YAML::Key << "Str" << YAML::Value << param->second.str;
		if (param->second.agi > 0)
			body << YAML::Key << "Agi" << YAML::Value << param->second.agi;
		if (param->second.vit > 0)
			body << YAML::Key << "Vit" << YAML::Value << param->second.vit;
		if (param->second.int_ > 0)
			body << YAML::Key << "Int" << YAML::Value << param->second.int_;
		if (param->second.dex > 0)
			body << YAML::Key << "Dex" << YAML::Value << param->second.dex;
		if (param->second.luk > 0)
			body << YAML::Key << "Luk" << YAML::Value << param->second.luk;

		body << YAML::EndMap;
	}
	body << YAML::EndMap;

	return true;
}

// elemental_db.yml function
//---------------------------
static bool read_elemental_skilldb( char* str[], size_t columns, size_t current ){
	uint16 skill_id = atoi(str[1]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("read_elemental_skilldb: Invalid skill '%hu'.\n", skill_id);
		return false;
	}

	uint16 skillmode = atoi(str[3]);

	std::string constant = constant_lookup(skillmode, "EL_SKILLMODE_");
	constant.erase(0, 13);
	std::string mode_name = name2Upper(constant);

	s_elemental_skill_csv entry = {};

	entry.skill_name = *skill_name;
	entry.lv = atoi(str[2]);
	entry.mode_name = mode_name;

	uint16 class_ = atoi(str[0]);

	if (util::umap_find(elemental_skill_tree, class_))
		elemental_skill_tree[class_].push_back(entry);
	else {
		elemental_skill_tree[class_] = std::vector<s_elemental_skill_csv>();
		elemental_skill_tree[class_].push_back(entry);
	}

	return true;
}

// Copied and adjusted from elemental.cpp
static bool read_elementaldb( char* str[], size_t columns, size_t current ){
	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << str[0];
	body << YAML::Key << "AegisName" << YAML::Value << str[1];
	body << YAML::Key << "Name" << YAML::Value << str[2];
	body << YAML::Key << "Level" << YAML::Value << str[3];
	if (atoi(str[4]) != 0)
		body << YAML::Key << "Hp" << YAML::Value << str[4];
	if (atoi(str[5]) != 1)
		body << YAML::Key << "Sp" << YAML::Value << str[5];
	if (atoi(str[7]) != 0)
		body << YAML::Key << "Attack" << YAML::Value << str[7];
	if (atoi(str[8]) != 0)
		body << YAML::Key << "Attack2" << YAML::Value << str[8];
	if (atoi(str[9]) != 0)
		body << YAML::Key << "Defense" << YAML::Value << str[9];
	if (atoi(str[10]) != 0)
		body << YAML::Key << "MagicDefense" << YAML::Value << str[10];
	if (atoi(str[11]) != 0)
		body << YAML::Key << "Str" << YAML::Value << str[11];
	if (atoi(str[12]) != 0)
		body << YAML::Key << "Agi" << YAML::Value << str[12];
	if (atoi(str[13]) != 0)
		body << YAML::Key << "Vit" << YAML::Value << str[13];
	if (atoi(str[14]) != 0)
		body << YAML::Key << "Int" << YAML::Value << str[14];
	if (atoi(str[15]) != 0)
		body << YAML::Key << "Dex" << YAML::Value << str[15];
	if (atoi(str[16]) != 0)
		body << YAML::Key << "Luk" << YAML::Value << str[16];
	if (atoi(str[6]) != 1)
		body << YAML::Key << "AttackRange" << YAML::Value << str[6];
	if (atoi(str[17]) != 5)
		body << YAML::Key << "SkillRange" << YAML::Value << str[17];
	if (atoi(str[18]) != 12)
		body << YAML::Key << "ChaseRange" << YAML::Value << str[18];
	body << YAML::Key << "Size" << YAML::Value << constant_lookup(strtol(str[19], nullptr, 10), "Size_") + 5;
	if (atoi(str[20]) != 0)
		body << YAML::Key << "Race" << YAML::Value << name2Upper(constant_lookup(atoi(str[20]), "RC_") + 3);

	int32 ele = strtol(str[21], nullptr, 10);
	body << YAML::Key << "Element" << YAML::Value << name2Upper(constant_lookup(ele % 20, "ELE_") + 4);
	body << YAML::Key << "ElementLevel" << YAML::Value << floor(ele / 20.);

	if (atoi(str[22]) != 200)
		body << YAML::Key << "WalkSpeed" << YAML::Value << str[22];
	if (atoi(str[23]) != 504)
		body << YAML::Key << "AttackDelay" << YAML::Value << str[23];
	if (atoi(str[24]) != 1020)
		body << YAML::Key << "AttackMotion" << YAML::Value << str[24];
	if (atoi(str[25]) != 360)
		body << YAML::Key << "DamageMotion" << YAML::Value << str[25];

	auto list = elemental_skill_tree.find( atoi(str[0]) );
	if (list != elemental_skill_tree.end()) {
		body << YAML::Key << "Mode";
			body << YAML::BeginMap;
		for (const auto &it : list->second) {
			body << YAML::Key << it.mode_name;
			body << YAML::BeginMap;
			body << YAML::Key << "Skill" << YAML::Value << it.skill_name;
			if (it.lv != 1)
				body << YAML::Key << "Level" << YAML::Value << it.lv;
			body << YAML::EndMap;
		}
			body << YAML::EndMap;
	}

	body << YAML::EndMap;

	return true;
}


// mercenary_db.yml function
//---------------------------
static bool mercenary_read_skilldb( char* str[], size_t columns, size_t current ){
	uint16 skill_id = atoi(str[1]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("mercenary_read_skilldb: Invalid skill '%hu'.\n", skill_id);
		return false;
	}

	s_mercenary_skill_csv entry = {};

	entry.skill_name = *skill_name;
	entry.max_lv = atoi(str[2]);

	uint16 class_ = atoi(str[0]);

	if (util::umap_find(mercenary_skill_tree, class_))
		mercenary_skill_tree[class_].push_back(entry);
	else {
		mercenary_skill_tree[class_] = std::vector<s_mercenary_skill_csv>();
		mercenary_skill_tree[class_].push_back(entry);
	}

	return true;
}

// Copied and adjusted from mercenary.cpp
static bool mercenary_readdb( char* str[], size_t columns, size_t current ){
	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << str[0];
	body << YAML::Key << "AegisName" << YAML::Value << str[1];
	body << YAML::Key << "Name" << YAML::Value << str[2];
	if (atoi(str[3]) != 1)
		body << YAML::Key << "Level" << YAML::Value << str[3];
	if (atoi(str[4]) != 1)
		body << YAML::Key << "Hp" << YAML::Value << str[4];
	if (atoi(str[5]) != 1)
		body << YAML::Key << "Sp" << YAML::Value << str[5];
	if (atoi(str[7]) != 0)
		body << YAML::Key << "Attack" << YAML::Value << str[7];
	if (atoi(str[8]) != 0)
		body << YAML::Key << "Attack2" << YAML::Value << str[8];
	if (atoi(str[9]) != 0)
		body << YAML::Key << "Defense" << YAML::Value << str[9];
	if (atoi(str[10]) != 0)
		body << YAML::Key << "MagicDefense" << YAML::Value << str[10];
	if (atoi(str[11]) != 1)
		body << YAML::Key << "Str" << YAML::Value << str[11];
	if (atoi(str[12]) != 1)
		body << YAML::Key << "Agi" << YAML::Value << str[12];
	if (atoi(str[13]) != 1)
		body << YAML::Key << "Vit" << YAML::Value << str[13];
	if (atoi(str[14]) != 1)
		body << YAML::Key << "Int" << YAML::Value << str[14];
	if (atoi(str[15]) != 1)
		body << YAML::Key << "Dex" << YAML::Value << str[15];
	if (atoi(str[16]) != 1)
		body << YAML::Key << "Luk" << YAML::Value << str[16];
	if (atoi(str[6]) != 0)
		body << YAML::Key << "AttackRange" << YAML::Value << str[6];
	if (atoi(str[17]) != 0)
		body << YAML::Key << "SkillRange" << YAML::Value << str[17];
	if (atoi(str[18]) != 0)
		body << YAML::Key << "ChaseRange" << YAML::Value << str[18];
	if (atoi(str[19]) != 0)
		body << YAML::Key << "Size" << YAML::Value << constant_lookup(strtol(str[19], nullptr, 10), "Size_") + 5;
	if (atoi(str[20]) != 0)
		body << YAML::Key << "Race" << YAML::Value << name2Upper(constant_lookup(atoi(str[20]), "RC_") + 3);

	int32 ele = strtol(str[21], nullptr, 10);
	if (atoi(str[21]) != 0)
		body << YAML::Key << "Element" << YAML::Value << name2Upper(constant_lookup(ele % 20, "ELE_") + 4);
	if (atoi(str[21]) != 1)
		body << YAML::Key << "ElementLevel" << YAML::Value << floor(ele / 20.);

	if (atoi(str[22]) != 0)
		body << YAML::Key << "WalkSpeed" << YAML::Value << cap_value(std::stoi(str[22]), MIN_WALK_SPEED, MAX_WALK_SPEED);
	if (atoi(str[23]) != 0)
		body << YAML::Key << "AttackDelay" << YAML::Value << str[23];
	if (atoi(str[24]) != 0)
		body << YAML::Key << "AttackMotion" << YAML::Value << str[24];
	if (atoi(str[25]) != 0)
		body << YAML::Key << "DamageMotion" << YAML::Value << str[25];

	for (const auto &skillit : mercenary_skill_tree) {
		if (skillit.first != atoi(str[0]))
			continue;

		body << YAML::Key << "Skills";
		body << YAML::BeginSeq;

		for (const auto &it : skillit.second) {
			body << YAML::BeginMap;
			body << YAML::Key << "Name" << YAML::Value << it.skill_name;
			body << YAML::Key << "MaxLevel" << YAML::Value << it.max_lv;
			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
	}

	body << YAML::EndMap;

	return true;
}

// Copied and adjusted from pc.cpp
static bool pc_readdb_skilltree( char* fields[], size_t columns, size_t current ){
	uint16 baselv, joblv, offset;
	uint16 class_  = (uint16)atoi(fields[0]);
	uint16 skill_id = (uint16)atoi(fields[1]);

	if (columns == 5 + MAX_PC_SKILL_REQUIRE * 2) { // Base/Job level requirement extra columns
		baselv = (uint16)atoi(fields[3]);
		joblv = (uint16)atoi(fields[4]);
		offset = 5;
	}
	else if (columns == 3 + MAX_PC_SKILL_REQUIRE * 2) {
		baselv = joblv = 0;
		offset = 3;
	}
	else {
		ShowWarning("pc_readdb_skilltree: Invalid number of colums in skill %hu of job %d's tree.\n", skill_id, class_);
		return false;
	}

	const char* constant = constant_lookup(class_, "JOB_");
	if (constant == nullptr) {
		ShowWarning("pc_readdb_skilltree: Invalid job class %d specified.\n", class_);
		return false;
	}
	std::string job_name(constant);

	std::string* skill_name = util::umap_find( aegis_skillnames, skill_id );

	if( skill_name == nullptr ){
		ShowWarning("pc_readdb_skilltree: Unable to load skill %hu into job %d's tree.\n", skill_id, class_);
		return false;
	}

	uint16 skill_lv = (uint16)atoi(fields[2]);

	std::vector<s_skill_tree_entry_csv> *job = util::map_find(skill_tree, class_);
	bool exists = job != nullptr;

	s_skill_tree_entry_csv entry;

	entry.skill_name = *skill_name;
	entry.skill_id = skill_id;
	entry.skill_lv = skill_lv;
	entry.baselv = baselv;
	entry.joblv = joblv;

	for (uint16 i = 0; i < MAX_PC_SKILL_REQUIRE; i++) {
		uint16 req_skill_id = (uint16)atoi(fields[i * 2 + offset]);
		skill_lv = (uint16)atoi(fields[i * 2 + offset + 1]);

		if (req_skill_id == 0)
			continue;

		skill_name = util::umap_find( aegis_skillnames, req_skill_id );

		if( skill_name == nullptr ){
			ShowWarning("pc_readdb_skilltree: Unable to load requirement skill %hu into job %d's tree.", req_skill_id, class_);
			return false;
		}
		entry.need.insert({ *skill_name, skill_lv });
	}

	if (exists)
		job->push_back(entry);
	else {
		std::vector<s_skill_tree_entry_csv> tree;

		tree.push_back(entry);
		skill_tree.insert({ class_, tree });
	}

	return true;
}

static bool pc_readdb_skilltree_yaml(void) {
	for (const auto &it : skill_tree) {
		body << YAML::BeginMap;
		std::string job = constant_lookup(it.first, "JOB_");
		job.erase( 0, 4 );
		body << YAML::Key << "Job" << YAML::Value << name2Upper( job );
		body << YAML::Key << "Tree";
		body << YAML::BeginSeq;
		for (const auto &subit : it.second) {
			body << YAML::BeginMap;
			body << YAML::Key << "Name" << YAML::Value << subit.skill_name;
			body << YAML::Key << "MaxLevel" << YAML::Value << subit.skill_lv;
			if (!subit.need.empty()) {
				body << YAML::Key << "Requires";
				body << YAML::BeginSeq;
				for (const auto &terit : subit.need) {
					body << YAML::BeginMap;
					body << YAML::Key << "Name" << YAML::Value << terit.first;
					body << YAML::Key << "Level" << YAML::Value << terit.second;
					body << YAML::EndMap;
				}
				body << YAML::EndSeq;
			}
			if (subit.baselv > 0)
				body << YAML::Key << "BaseLevel" << YAML::Value << subit.baselv;
			if (subit.joblv > 0)
				body << YAML::Key << "JobLevel" << YAML::Value << subit.joblv;
			body << YAML::EndMap;
		}
		body << YAML::EndSeq;
		body << YAML::EndMap;
	}
	return true;
}

// Copied and adjusted from itemdb.cpp
static int32 itemdb_combo_split_atoi (char *str, t_itemid *val) {
	int32 i;

	for (i = 0; i < MAX_ITEMS_PER_COMBO; i++) {
		if (!str)
			break;
		val[i] = strtoul(str, nullptr, 10);
		str = strchr(str,':');
		if (str)
			*str++=0;
	}
	if (i == 0) //No data found.
		return 0;

	return i;
}

// Copied and adjusted from itemdb.cpp
static bool itemdb_read_combos(const char* file) {
	FILE* fp = fopen( file, "r" );

	if( fp == nullptr ){
		ShowError( "Can't read %s\n", file );
		return 0;
	}

	uint32 lines = 0, count = 0;
	char line[1024];
	char path[256];
	
	// process rows one by one
	while (fgets(line, sizeof(line), fp)) {
		char *str[2], *p;

		lines++;

		if (line[0] == '/' && line[1] == '/')
			continue;

		memset(str, 0, sizeof(str));

		p = line;

		p = trim(p);

		if (*p == '\0')
			continue;// empty line

		if (!strchr(p,',')) {
			ShowError("itemdb_read_combos: Insufficient columns in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}

		str[0] = p;
		p = strchr(p,',');
		*p = '\0';
		p++;

		str[1] = p;
		p = strchr(p,',');
		p++;

		if (str[1][0] != '{') {
			ShowError("itemdb_read_combos(#1): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}
		if ( str[1][strlen(str[1])-1] != '}' ) {
			ShowError("itemdb_read_combos(#2): Invalid format (Script column) in line %d of \"%s\", skipping.\n", lines, path);
			continue;
		}
		t_itemid items[MAX_ITEMS_PER_COMBO];
		int32 v = 0, retcount = 0;

		if ((retcount = itemdb_combo_split_atoi(str[0], items)) < 2) {
			ShowError("itemdb_read_combos: line %d of \"%s\" doesn't have enough items to make for a combo (min:2), skipping.\n", lines, path);
			continue;
		}
		for (v = 0; v < retcount; v++) {
			std::string *item_name = util::umap_find(aegis_itemnames, items[v]);

			if (item_name == nullptr) {
				ShowWarning("itemdb_read_combos: Invalid item ID %" PRIu32 ".\n", items[v]);
				break;
			}
		}
		if (v < retcount)
			continue;

		body << YAML::BeginMap;
		body << YAML::Key << "Combos";
		body << YAML::BeginSeq;

		body << YAML::BeginMap;
		body << YAML::Key << "Combo";
		body << YAML::BeginSeq;

		for (v = 0; v < retcount; v++) {
			body << YAML::BeginMap;
			std::string *item_name = util::umap_find(aegis_itemnames, items[v]);
			body << YAML::Key << *item_name;
			body << YAML::EndMap;
		}
		body << YAML::EndSeq;
		body << YAML::EndMap;

		str[1] = str[1] + 1;	//skip the first left curly
		str[1][strlen(str[1])-1] = '\0';	//null the last right curly

		body << YAML::EndSeq;
		body << YAML::Key << "Script" << YAML::Literal << trim(str[1]);
		body << YAML::EndMap;

		count++;
	}
	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, file);

	return true;
}

// Copied and adjusted from cashshop.cpp
static bool cashshop_parse_dbrow( char* fields[], size_t columns, size_t current ){
	uint16 tab = atoi( fields[0] );
	t_itemid nameid = strtoul( fields[1], nullptr, 10 );
	uint32 price = atoi( fields[2] );

	std::string* item_name = util::umap_find( aegis_itemnames, nameid );

	if( item_name == nullptr ){
		ShowWarning( "cashshop_parse_dbrow: Invalid item id %u.\n", nameid );
		return false;
	}

	if( tab >= CASHSHOP_TAB_MAX ){
		ShowWarning( "cashshop_parse_dbrow: Invalid tab %d in line '%d', skipping...\n", tab, current );
		return false;
	}else if( price < 1 ){
		ShowWarning( "cashshop_parse_dbrow: Invalid price %d in line '%d', skipping...\n", price, current );
		return false;
	}

	const char* constant_ptr = constant_lookup( tab, "CASHSHOP_TAB_" );

	if( constant_ptr == nullptr ){
		ShowError( "cashshop_parse_dbrow: CASHSHOP_TAB constant for tab %hu was not found, skipping...\n", tab );
		return false;
	}

	std::string constant = constant_ptr;
	constant.erase( 0, 13 );
	constant = name2Upper( constant );

	body << YAML::BeginMap;
	body << YAML::Key << "Tab" << YAML::Value << constant;
	body << YAML::Key << "Items";
	body << YAML::BeginSeq;
	body << YAML::BeginMap;
	body << YAML::Key << "Item" << YAML::Value << *item_name;
	body << YAML::Key << "Price" << YAML::Value << price;
	body << YAML::EndMap;
	body << YAML::EndSeq;
	body << YAML::EndMap;

	return true;
}

// homunculus_db.yml function
//---------------------------
static bool read_homunculus_skilldb( char* split[], size_t columns, size_t current ){
	s_homun_skill_tree_entry entry = {};

	entry.id = atoi(split[1]);
	entry.max = atoi(split[2]);
	entry.need_level = atoi(split[3]);
	entry.intimacy = cap_value(atoi(split[14]), 0, 1000);

	for (int32 i = 0; i < MAX_HOM_SKILL_REQUIRE; i++) {
		if (atoi(split[4 + i * 2]) > 0)
			entry.need.emplace(atoi(split[4 + i * 2]), atoi(split[4 + i * 2 + 1]));
	}

	if (util::umap_find(hom_skill_tree, atoi(split[0])))
		hom_skill_tree[(uint16)atoi(split[0])].push_back(entry);
	else {
		hom_skill_tree[(uint16)atoi(split[0])] = std::vector<s_homun_skill_tree_entry>();
		hom_skill_tree[(uint16)atoi(split[0])].push_back(entry);
	}

	return true;
}

static bool compareHomSkillId(const s_homun_skill_tree_entry &a, const s_homun_skill_tree_entry &b) {
	return a.id < b.id;
}

// Copied and adjusted from homunculus.cpp
static bool read_homunculusdb( char* str[], size_t columns, size_t current ){
	bool has_evo = false;

	body << YAML::BeginMap;
	body << YAML::Key << "Class" << YAML::Value << name2Upper(constant_lookup(atoi(str[0]), "MER_") + 4);
	body << YAML::Key << "Name" << YAML::Value << str[2];
	if (atoi(str[0]) != atoi(str[1])) {
		body << YAML::Key << "EvolutionClass" << YAML::Value << name2Upper(constant_lookup(atoi(str[1]), "MER_") + 4);
		has_evo = true;
	}
	if (atoi(str[3]) != ITEMID_PET_FOOD)
		body << YAML::Key << "Food" << YAML::Value << name2Upper(*util::umap_find(aegis_itemnames, (t_itemid)atoi(str[3])));
	if (atoi(str[4]) != 60000)
		body << YAML::Key << "HungryDelay" << YAML::Value << atoi(str[4]);

	if (atoi(str[7]) != RC_DEMIHUMAN)
		body << YAML::Key << "Race" << YAML::Value << name2Upper(constant_lookup(atoi(str[7]), "RC_") + 3);
	if (atoi(str[8]) != ELE_NEUTRAL)
	body << YAML::Key << "Element" << YAML::Value << name2Upper(constant_lookup(atoi(str[8]), "ELE_") + 4);
	if (atoi(str[5]) != SZ_SMALL)
		body << YAML::Key << "Size" << YAML::Value << name2Upper(constant_lookup(atoi(str[5]), "Size_") + 5);
	if (atoi(str[6]) != SZ_MEDIUM)
		body << YAML::Key << "EvolutionSize" << YAML::Value << name2Upper(constant_lookup(atoi(str[6]), "Size_") + 5);
	if (atoi(str[9]) != 700)
		body << YAML::Key << "AttackDelay" << YAML::Value << atoi(str[9]);

	body << YAML::Key << "Status";
	body << YAML::BeginSeq;

	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << "Hp";
	body << YAML::Key << "Base" << YAML::Value << atoi(str[10]);
	body << YAML::Key << "GrowthMinimum" << YAML::Value << atoi(str[18]);
	body << YAML::Key << "GrowthMaximum" << YAML::Value << atoi(str[19]);
	if (has_evo) {
		body << YAML::Key << "EvolutionMinimum" << YAML::Value << atoi(str[34]);
		body << YAML::Key << "EvolutionMaximum" << YAML::Value << atoi(str[35]);
	}
	body << YAML::EndMap;

	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << "Sp";
	body << YAML::Key << "Base" << YAML::Value << atoi(str[11]);
	body << YAML::Key << "GrowthMinimum" << YAML::Value << atoi(str[20]);
	body << YAML::Key << "GrowthMaximum" << YAML::Value << atoi(str[21]);
	if (has_evo) {
		body << YAML::Key << "EvolutionMinimum" << YAML::Value << atoi(str[36]);
		body << YAML::Key << "EvolutionMaximum" << YAML::Value << atoi(str[37]);
	}
	body << YAML::EndMap;

	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << "Str";
	body << YAML::Key << "Base" << YAML::Value << atoi(str[12]);
	body << YAML::Key << "GrowthMinimum" << YAML::Value << atoi(str[22]);
	body << YAML::Key << "GrowthMaximum" << YAML::Value << atoi(str[23]);
	if (has_evo) {
		body << YAML::Key << "EvolutionMinimum" << YAML::Value << atoi(str[38]);
		body << YAML::Key << "EvolutionMaximum" << YAML::Value << atoi(str[39]);
	}
	body << YAML::EndMap;

	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << "Agi";
	body << YAML::Key << "Base" << YAML::Value << atoi(str[13]);
	body << YAML::Key << "GrowthMinimum" << YAML::Value << atoi(str[24]);
	body << YAML::Key << "GrowthMaximum" << YAML::Value << atoi(str[25]);
	if (has_evo) {
		body << YAML::Key << "EvolutionMinimum" << YAML::Value << atoi(str[40]);
		body << YAML::Key << "EvolutionMaximum" << YAML::Value << atoi(str[41]);
	}
	body << YAML::EndMap;

	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << "Vit";
	body << YAML::Key << "Base" << YAML::Value << atoi(str[14]);
	body << YAML::Key << "GrowthMinimum" << YAML::Value << atoi(str[26]);
	body << YAML::Key << "GrowthMaximum" << YAML::Value << atoi(str[27]);
	if (has_evo) {
		body << YAML::Key << "EvolutionMinimum" << YAML::Value << atoi(str[42]);
		body << YAML::Key << "EvolutionMaximum" << YAML::Value << atoi(str[43]);
	}
	body << YAML::EndMap;

	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << "Int";
	body << YAML::Key << "Base" << YAML::Value << atoi(str[15]);
	body << YAML::Key << "GrowthMinimum" << YAML::Value << atoi(str[28]);
	body << YAML::Key << "GrowthMaximum" << YAML::Value << atoi(str[29]);
	if (has_evo) {
		body << YAML::Key << "EvolutionMinimum" << YAML::Value << atoi(str[44]);
		body << YAML::Key << "EvolutionMaximum" << YAML::Value << atoi(str[45]);
	}
	body << YAML::EndMap;

	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << "Dex";
	body << YAML::Key << "Base" << YAML::Value << atoi(str[16]);
	body << YAML::Key << "GrowthMinimum" << YAML::Value << atoi(str[30]);
	body << YAML::Key << "GrowthMaximum" << YAML::Value << atoi(str[31]);
	if (has_evo) {
		body << YAML::Key << "EvolutionMinimum" << YAML::Value << atoi(str[46]);
		body << YAML::Key << "EvolutionMaximum" << YAML::Value << atoi(str[47]);
	}
	body << YAML::EndMap;

	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << "Luk";
	body << YAML::Key << "Base" << YAML::Value << atoi(str[17]);
	body << YAML::Key << "GrowthMinimum" << YAML::Value << atoi(str[32]);
	body << YAML::Key << "GrowthMaximum" << YAML::Value << atoi(str[33]);
	if (has_evo) {
		body << YAML::Key << "EvolutionMinimum" << YAML::Value << atoi(str[48]);
		body << YAML::Key << "EvolutionMaximum" << YAML::Value << atoi(str[49]);
	}
	body << YAML::EndMap;

	body << YAML::EndSeq;

	// Gather and sort skill tree data
	std::vector<s_homun_skill_tree_entry> *skill_tree_base = nullptr, *skill_tree_evo = nullptr;

	skill_tree_base = util::umap_find(hom_skill_tree, atoi(str[0]));
	std::sort(skill_tree_base->begin(), skill_tree_base->end(), compareHomSkillId);

	if (has_evo) {
		skill_tree_evo = util::umap_find(hom_skill_tree, atoi(str[1]));
		std::sort(skill_tree_evo->begin(), skill_tree_evo->end(), compareHomSkillId);
	}

	// Get a difference between the base class and evo class skill tree to find skills granted through evolution
	std::vector<s_homun_skill_tree_entry> tree_diff;

	if (skill_tree_evo != nullptr) {
		for (const auto &evoit : *skill_tree_evo) {
			bool contains = false;

			for (const auto &baseit : *skill_tree_base) {
				if (baseit.id == evoit.id) {
					contains = true;
					break;
				}
			}

			if (!contains) {
				// Skill is not part of the base tree, we can only assume it's an evolution granted skill thanks to brilliant formatting of our TXT homun_db!
				tree_diff.push_back(evoit);
			}
		}
	}

	if (skill_tree_base != nullptr) {
		body << YAML::Key << "SkillTree";
		body << YAML::BeginSeq;

		for (const auto &skillit : *skill_tree_base) {
			std::string *skill_name = util::umap_find(aegis_skillnames, skillit.id);

			if (skill_name == nullptr) {
				ShowError("Skill name for homunculus skill ID %hu is not known.\n", skillit.id);
				return false;
			}

			body << YAML::BeginMap;
			body << YAML::Key << "Skill" << YAML::Value << *skill_name;
			body << YAML::Key << "MaxLevel" << YAML::Value << (int32)skillit.max;
			if (skillit.need_level > 0)
				body << YAML::Key << "RequiredLevel" << YAML::Value << (int32)skillit.need_level;
			if (skillit.intimacy > 0)
				body << YAML::Key << "RequiredIntimacy" << YAML::Value << skillit.intimacy;

			if (!skillit.need.empty()) {
				body << YAML::Key << "Required";
				body << YAML::BeginSeq;

				for (const auto &it : skillit.need) {
					uint16 required_skill_id = it.first;
					uint16 required_skill_level = it.second;

					if (required_skill_id == 0 || required_skill_level == 0)
						continue;

					std::string *required_name = util::umap_find(aegis_skillnames, required_skill_id);

					if (required_name == nullptr) {
						ShowError("Skill name for required skill id %hu is not known.\n", required_skill_id);
						return false;
					}

					body << YAML::BeginMap;
					body << YAML::Key << "Skill" << YAML::Value << *required_name;
					body << YAML::Key << "Level" << YAML::Value << required_skill_level;
					body << YAML::EndMap;
				}

				body << YAML::EndSeq;
			}

			body << YAML::EndMap;
		}

		for (const auto &skillit : tree_diff) {
			std::string *skill_name = util::umap_find(aegis_skillnames, skillit.id);

			if (skill_name == nullptr) {
				ShowError("Skill name for homunculus skill ID %hu is not known.\n", skillit.id);
				return false;
			}

			body << YAML::BeginMap;
			body << YAML::Key << "Skill" << YAML::Value << *skill_name;
			body << YAML::Key << "MaxLevel" << YAML::Value << (int32)skillit.max;
			if (skillit.need_level > 0)
				body << YAML::Key << "RequiredLevel" << YAML::Value << (int32)skillit.need_level;
			if (skillit.intimacy > 0)
				body << YAML::Key << "RequiredIntimacy" << YAML::Value << skillit.intimacy;
			body << YAML::Key << "RequireEvolution" << YAML::Value << "true";

			if (!skillit.need.empty()) {
				body << YAML::Key << "Required";
				body << YAML::BeginSeq;

				for (const auto &it : skillit.need) {
					uint16 required_skill_id = it.first;
					uint16 required_skill_level = it.second;

					if (required_skill_id == 0 || required_skill_level == 0)
						continue;

					std::string *required_name = util::umap_find(aegis_skillnames, required_skill_id);

					if (required_name == nullptr) {
						ShowError("Skill name for required skill id %hu is not known.\n", required_skill_id);
						return false;
					}

					body << YAML::BeginMap;
					body << YAML::Key << "Skill" << YAML::Value << *required_name;
					body << YAML::Key << "Level" << YAML::Value << required_skill_level;
					body << YAML::EndMap;
				}

				body << YAML::EndSeq;
			}

			body << YAML::EndMap;
		}

		body << YAML::EndSeq;
	}

	body << YAML::EndMap;

	return true;
}

int32 main( int32 argc, char *argv[] ){
	return main_core<Csv2YamlTool>( argc, argv );
}
