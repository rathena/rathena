// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
#include <map>
#include <unordered_map>
#include <vector>

#ifdef WIN32
	#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
	#include <stdio.h>
#endif

#include <yaml-cpp/yaml.h>

#include "../common/cbasetypes.hpp"
#include "../common/core.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/utilities.hpp"
#include "../common/utils.hpp"
#ifdef WIN32
#include "../common/winapi.hpp"
#endif

// Only for constants - do not use functions of it or linking will fail
#include "../map/achievement.hpp"
#include "../map/battle.hpp"
#include "../map/battleground.hpp"
#include "../map/channel.hpp"
#include "../map/chat.hpp"
#include "../map/date.hpp"
#include "../map/instance.hpp"
#include "../map/mercenary.hpp"
#include "../map/mob.hpp"
#include "../map/npc.hpp"
#include "../map/pc.hpp"
#include "../map/pet.hpp"
#include "../map/quest.hpp"
#include "../map/script.hpp"
#include "../map/skill.hpp"
#include "../map/storage.hpp"

using namespace rathena;

#define MAX_MAP_PER_INSTANCE 255

#ifndef WIN32
int getch( void ){
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
#endif

// Required constant and structure definitions
#define MAX_GUILD_SKILL_REQUIRE 5
#define MAX_SKILL_ITEM_REQUIRE	10
#define MAX_SKILL_STATUS_REQUIRE 3
#define MAX_SKILL_EQUIP_REQUIRE 10
#define MAX_QUEST_DROPS 3

struct s_skill_unit_csv : s_skill_db {
	std::string target_str;
	int unit_flag_csv;
};

std::unordered_map<uint16, s_skill_require> skill_require;
std::unordered_map<uint16, s_skill_db> skill_cast;
std::unordered_map<uint16, s_skill_db> skill_castnodex;
std::unordered_map<uint16, s_skill_unit_csv> skill_unit;
std::unordered_map<uint16, s_skill_copyable> skill_copyable;
std::unordered_map<uint16, s_skill_db> skill_nearnpc;

static unsigned int level_penalty[3][CLASS_MAX][MAX_LEVEL * 2 + 1];

struct s_item_flag_csv2yaml {
	bool buyingstore, dead_branch, group, guid, broadcast, bindOnEquip, delay_consume;
	e_item_drop_effect dropEffect;
};

struct s_item_delay_csv2yaml {
	uint32 delay;
	std::string sc;
};

struct s_item_stack_csv2yaml {
	uint16 amount;
	bool inventory, cart, storage, guild_storage;
};

struct s_item_nouse_csv2yaml {
	uint16 override;
	bool sitting;
};

struct s_item_trade_csv2yaml {
	uint16 override;
	bool drop, trade, trade_partner, sell, cart, storage, guild_storage, mail, auction;
};

std::unordered_map<t_itemid, t_itemid> item_avail;
std::unordered_map<t_itemid, bool> item_buyingstore;
std::unordered_map<t_itemid, s_item_flag_csv2yaml> item_flag;
std::unordered_map<t_itemid, s_item_delay_csv2yaml> item_delay;
std::unordered_map<t_itemid, s_item_stack_csv2yaml> item_stack;
std::unordered_map<t_itemid, s_item_nouse_csv2yaml> item_nouse;
std::unordered_map<t_itemid, s_item_trade_csv2yaml> item_trade;

struct s_random_opt_group_csv : s_random_opt_group {
	std::vector<uint16> rate;
};

std::unordered_map<uint16, std::string> rand_opt_db;
std::unordered_map<uint16, s_random_opt_group_csv> rand_opt_group;

static std::map<std::string, int> um_mapid2jobname {
	{ "Novice", JOB_NOVICE }, // Novice and Super Novice share the same value
	{ "SuperNovice", JOB_NOVICE },
	{ "Swordman", JOB_SWORDMAN },
	{ "Mage", JOB_MAGE },
	{ "Archer", JOB_ARCHER },
	{ "Acolyte", JOB_ACOLYTE },
	{ "Merchant", JOB_MERCHANT },
	{ "Thief", JOB_THIEF },
	{ "Knight", JOB_KNIGHT },
	{ "Priest", JOB_PRIEST },
	{ "Wizard", JOB_WIZARD },
	{ "Blacksmith", JOB_BLACKSMITH },
	{ "Hunter", JOB_HUNTER },
	{ "Assassin", JOB_ASSASSIN },
	{ "Crusader", JOB_CRUSADER },
	{ "Monk", JOB_MONK },
	{ "Sage", JOB_SAGE },
	{ "Rogue", JOB_ROGUE },
	{ "Alchemist", JOB_ALCHEMIST },
	{ "BardDancer", JOB_BARD }, // Bard and Dancer share the same value
	{ "BardDancer", JOB_DANCER },
	{ "Gunslinger", JOB_GUNSLINGER },
	{ "Ninja", JOB_NINJA },
	{ "Taekwon", 21 },
	{ "StarGladiator", 22 },
	{ "SoulLinker", 23 },
//	{ "Gangsi", 26 },
//	{ "DeathKnight", 27 },
//	{ "DarkCollector", 28 },
#ifdef RENEWAL
	{ "KagerouOboro", 29 }, // Kagerou and Oboro share the same value
	{ "Rebellion", 30 },
	{ "Summoner", 31 },
#endif
};

static std::unordered_map<std::string, equip_pos> um_equipnames {
	{ "Head_Low", EQP_HEAD_LOW },
	{ "Head_Mid", EQP_HEAD_MID },
	{ "Head_Top", EQP_HEAD_TOP },
	{ "Right_Hand", EQP_HAND_R },
	{ "Left_Hand", EQP_HAND_L },
	{ "Armor", EQP_ARMOR },
	{ "Shoes", EQP_SHOES },
	{ "Garment", EQP_GARMENT },
	{ "Right_Accessory", EQP_ACC_R },
	{ "Left_Accessory", EQP_ACC_L },
	{ "Costume_Head_Top", EQP_COSTUME_HEAD_TOP },
	{ "Costume_Head_Mid", EQP_COSTUME_HEAD_MID },
	{ "Costume_Head_Low", EQP_COSTUME_HEAD_LOW },
	{ "Costume_Garment", EQP_COSTUME_GARMENT },
	{ "Ammo", EQP_AMMO },
	{ "Shadow_Armor", EQP_SHADOW_ARMOR },
	{ "Shadow_Weapon", EQP_SHADOW_WEAPON },
	{ "Shadow_Shield", EQP_SHADOW_SHIELD },
	{ "Shadow_Shoes", EQP_SHADOW_SHOES },
	{ "Shadow_Right_Accessory", EQP_SHADOW_ACC_R },
	{ "Shadow_Left_Accessory", EQP_SHADOW_ACC_L },
};

// Forward declaration of conversion functions
static bool guild_read_guildskill_tree_db( char* split[], int columns, int current );
static bool pet_read_db( const char* file );
static bool skill_parse_row_magicmushroomdb(char *split[], int column, int current);
static bool skill_parse_row_abradb(char* split[], int columns, int current);
static bool skill_parse_row_spellbookdb(char* split[], int columns, int current);
static bool mob_readdb_mobavail(char *str[], int columns, int current);
static bool skill_parse_row_requiredb(char* split[], int columns, int current);
static bool skill_parse_row_castdb(char* split[], int columns, int current);
static bool skill_parse_row_castnodexdb(char* split[], int columns, int current);
static bool skill_parse_row_unitdb(char* split[], int columns, int current);
static bool skill_parse_row_copyabledb(char* split[], int columns, int current);
static bool skill_parse_row_nonearnpcrangedb(char* split[], int columns, int current);
static bool skill_parse_row_skilldb(char* split[], int columns, int current);
static bool quest_read_db(char *split[], int columns, int current);
static bool instance_readdb_sub(char* str[], int columns, int current);
static bool itemdb_read_itemavail(char *str[], int columns, int current);
static bool itemdb_read_buyingstore(char* fields[], int columns, int current);
static bool itemdb_read_flag(char* fields[], int columns, int current);
static bool itemdb_read_itemdelay(char* str[], int columns, int current);
static bool itemdb_read_stack(char* fields[], int columns, int current);
static bool itemdb_read_nouse(char* fields[], int columns, int current);
static bool itemdb_read_itemtrade(char* fields[], int columns, int current);
static bool itemdb_read_db(const char *file);
static bool itemdb_read_randomopt(const char* file);
static bool itemdb_read_randomopt_group(char *str[], int columns, int current);
static bool itemdb_randomopt_group_yaml(void);
static bool pc_readdb_levelpenalty(char* fields[], int columns, int current);
static bool pc_levelpenalty_yaml();

// Constants for conversion
std::unordered_map<t_itemid, std::string> aegis_itemnames;
std::unordered_map<t_itemid, t_itemid> aegis_itemviewid;
std::unordered_map<uint16, std::string> aegis_mobnames;
std::unordered_map<uint16, std::string> aegis_skillnames;
std::unordered_map<const char*, int64> constants;

// Forward declaration of constant loading functions
static bool parse_item_constants_txt( const char* path );
static bool parse_mob_constants( char* split[], int columns, int current );
static bool parse_skill_constants_txt( char* split[], int columns, int current );
static void init_random_option_constants();

bool fileExists( const std::string& path );
bool askConfirmation( const char* fmt, ... );

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

YAML::Emitter body;

// Implement the function instead of including the original version by linking
void script_set_constant_( const char* name, int64 value, const char* constant_name, bool isparameter, bool deprecated ){
	constants[name] = value;
}

const char* constant_lookup( int32 value, const char* prefix ){
	if (prefix == nullptr)
		return nullptr;

	for( auto const& pair : constants ){
		// Same prefix group and same value
		if( strncasecmp( pair.first, prefix, strlen( prefix ) ) == 0 && pair.second == value ){
			return pair.first;
		}
	}

	return nullptr;
}

int64 constant_lookup_int(const char* constant) {
	if (constant == nullptr)
		return -100;

	for (auto const &pair : constants) {
		if (strlen(pair.first) == strlen(constant) && strncasecmp(pair.first, constant, strlen(constant)) == 0) {
			return pair.second;
		}
	}

	return -100;
}

void copyFileIfExists( std::ofstream& file,const std::string& name, bool newLine ){
	std::string path = "doc/yaml/db/" + name + ".yml";

	if( fileExists( path ) ){
		std::ifstream source( path, std::ios::binary );

		std::istreambuf_iterator<char> begin_source( source );
		std::istreambuf_iterator<char> end_source;
		std::ostreambuf_iterator<char> begin_dest( file );
		copy( begin_source, end_source, begin_dest );

		source.close();

		if( newLine ){
			file << "\n";
		}
	}
}

void prepareHeader(std::ofstream &file, const std::string& type, uint32 version, const std::string& name) {
	copyFileIfExists( file, "license", false );
	copyFileIfExists( file, name, true );

	YAML::Emitter header(file);

	header << YAML::BeginMap;
	header << YAML::Key << "Header";
	header << YAML::BeginMap;
	header << YAML::Key << "Type" << YAML::Value << type;
	header << YAML::Key << "Version" << YAML::Value << version;
	header << YAML::EndMap;
	header << YAML::EndMap;

	file << "\n";
	file << "\n";
}

void prepareBody(void) {
	body << YAML::BeginMap;
	body << YAML::Key << "Body";
	body << YAML::BeginSeq;
}

void finalizeBody(void) {
	body << YAML::EndSeq;
	body << YAML::EndMap;
}

template<typename Func>
bool process( const std::string& type, uint32 version, const std::vector<std::string>& paths, const std::string& name, Func lambda, const std::string& rename = "" ){
	for( const std::string& path : paths ){
		const std::string name_ext = name + ".txt";
		const std::string from = path + "/" + name_ext;
		const std::string to = path + "/" + (rename.size() > 0 ? rename : name) + ".yml";

		if( fileExists( from ) ){
			if( !askConfirmation( "Found the file \"%s\", which requires migration to yml.\nDo you want to convert it now? (Y/N)\n", from.c_str() ) ){
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

			prepareHeader(out, type, version, (rename.size() > 0 ? rename : name));
			prepareBody();

			if( !lambda( path, name_ext ) ){
				out.close();
				return false;
			}

			finalizeBody();
			out << body.c_str();
			// Make sure there is an empty line at the end of the file for git
			out << "\n";
			out.close();
			
			// TODO: do you want to delete?
		}
	}

	return true;
}

int do_init( int argc, char** argv ){
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
	sv_readdb( path_db_mode.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants, false );
	sv_readdb( path_db_import.c_str(), "mob_db.txt", ',', 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, 31 + 2 * MAX_MVP_DROP + 2 * MAX_MOB_DROP, -1, &parse_mob_constants, false );
	if (fileExists(skill_db.getDefaultLocation())) {
		skill_db.load();
	} else {
		sv_readdb(path_db_mode.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants_txt, false);
		sv_readdb(path_db_import.c_str(), "skill_db.txt", ',', 18, 18, -1, parse_skill_constants_txt, false);
	}

	// Load constants
	#define export_constant_npc(a) export_constant(a)
	init_random_option_constants();
	#include "../map/script_constants.hpp"

	std::vector<std::string> root_paths = {
		path_db,
		path_db_mode,
		path_db_import
	};

	if( !process( "GUILD_SKILL_TREE_DB", 1, root_paths, "guild_skill_tree", []( const std::string& path, const std::string& name_ext ) -> bool {
		return sv_readdb( path.c_str(), name_ext.c_str(), ',', 2 + MAX_GUILD_SKILL_REQUIRE * 2, 2 + MAX_GUILD_SKILL_REQUIRE * 2, -1, &guild_read_guildskill_tree_db, false );
	} ) ){
		return 0;
	}

	if( !process( "PET_DB", 1, root_paths, "pet_db", []( const std::string& path, const std::string& name_ext ) -> bool {
		return pet_read_db( ( path + name_ext ).c_str() );
	} ) ){
		return 0;
	}

	if (!process("MAGIC_MUSHROOM_DB", 1, root_paths, "magicmushroom_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 1, 1, -1, &skill_parse_row_magicmushroomdb, false);
	})) {
		return 0;
	}

	if (!process("ABRA_DB", 1, root_paths, "abra_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 3, 3, -1, &skill_parse_row_abradb, false);
	})) {
		return 0;
	}

	if (!process("READING_SPELLBOOK_DB", 1, root_paths, "spellbook_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 3, 3, -1, &skill_parse_row_spellbookdb, false);
	})) {
		return 0;
	}

	if (!process("MOB_AVAIL_DB", 1, root_paths, "mob_avail", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 2, 12, -1, &mob_readdb_mobavail, false);
	})) {
		return 0;
	}

	skill_txt_data( path_db_mode, path_db );
	if (!process("SKILL_DB", 1, { path_db_mode }, "skill_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 18, 18, -1, &skill_parse_row_skilldb, false);
	})){
		return 0;
	}

	skill_txt_data( path_db_import, path_db_import );
	if (!process("SKILL_DB", 1, { path_db_import }, "skill_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 18, 18, -1, &skill_parse_row_skilldb, false);
	})){
		return 0;
	}

	if (!process("QUEST_DB", 1, root_paths, "quest_db", [](const std::string &path, const std::string &name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 3 + MAX_QUEST_OBJECTIVES * 2 + MAX_QUEST_DROPS * 3, 100, -1, &quest_read_db, false);
	})) {
		return 0;
	}

	if (!process("INSTANCE_DB", 1, root_paths, "instance_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 7, 7 + MAX_MAP_PER_INSTANCE, -1, &instance_readdb_sub, false);
	})) {
		return 0;
	}

	item_txt_data(path_db_mode, path_db);
	if (!process("ITEM_DB", 1, { path_db_mode }, "item_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return itemdb_read_db((path + name_ext).c_str());
	})) {
		return 0;
	}

	item_txt_data(path_db_import, path_db_import);
	if (!process("ITEM_DB", 1, { path_db_import }, "item_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return itemdb_read_db((path + name_ext).c_str());
	})) {
		return 0;
	}

	rand_opt_db.clear();
	if (!process("RANDOM_OPTION_DB", 1, root_paths, "item_randomopt_db", [](const std::string& path, const std::string& name_ext) -> bool {
		return itemdb_read_randomopt((path + name_ext).c_str());
	})) {
		return 0;
	}

	rand_opt_group.clear();
	if (!process("RANDOM_OPTION_GROUP", 1, root_paths, "item_randomopt_group", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 5, 2 + 5 * MAX_ITEM_RDM_OPT, -1, &itemdb_read_randomopt_group, false) && itemdb_randomopt_group_yaml();
	})) {
		return 0;
	}

#ifdef RENEWAL
	memset( level_penalty, 0, sizeof( level_penalty ) );
	if (!process("PENALTY_DB", 1, { path_db_mode }, "level_penalty", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 4, 4, -1, &pc_readdb_levelpenalty, false) && pc_levelpenalty_yaml();
	})) {
		return 0;
	}

	memset( level_penalty, 0, sizeof( level_penalty ) );
	if (!process("PENALTY_DB", 1, { path_db_import }, "level_penalty", [](const std::string& path, const std::string& name_ext) -> bool {
		return sv_readdb(path.c_str(), name_ext.c_str(), ',', 4, 4, -1, &pc_readdb_levelpenalty, false) && pc_levelpenalty_yaml();
	})) {
		return 0;
	}
#endif

	// TODO: add implementations ;-)

	return 0;
}

void do_final(void){
}

bool fileExists( const std::string& path ){
	std::ifstream in;

	in.open( path );

	if( in.is_open() ){
		in.close();

		return true;
	}else{
		return false;
	}
}

bool askConfirmation( const char* fmt, ... ){
	va_list ap;

	va_start( ap, fmt );

	_vShowMessage( MSG_NONE, fmt, ap );

	va_end( ap );

	char c = getch();

	if( c == 'Y' || c == 'y' ){
		return true;
	}else{
		return false;
	}
}

// Constant loading functions
static bool parse_item_constants_txt( const char* path ){
	uint32 lines = 0, count = 0;
	char line[1024];

	FILE* fp;

	fp = fopen(path, "r");
	if (fp == NULL) {
		ShowWarning("parse_item_constants_txt: File not found \"%s\", skipping.\n", path);
		return false;
	}

	// process rows one by one
	while (fgets(line, sizeof(line), fp))
	{
		char *str[32], *p;
		int i;
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
		for (i = 0; i < 19; ++i)
		{
			str[i] = p;
			p = strchr(p, ',');
			if (p == NULL)
				break;// comma not found
			*p = '\0';
			++p;
		}

		if (p == NULL)
		{
			ShowError("parse_item_constants_txt: Insufficient columns in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}

		// Script
		if (*p != '{')
		{
			ShowError("parse_item_constants_txt: Invalid format (Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}
		str[19] = p + 1;
		p = strstr(p + 1, "},");
		if (p == NULL)
		{
			ShowError("parse_item_constants_txt: Invalid format (Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}
		*p = '\0';
		p += 2;

		// OnEquip_Script
		if (*p != '{')
		{
			ShowError("parse_item_constants_txt: Invalid format (OnEquip_Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}
		str[20] = p + 1;
		p = strstr(p + 1, "},");
		if (p == NULL)
		{
			ShowError("parse_item_constants_txt: Invalid format (OnEquip_Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}
		*p = '\0';
		p += 2;

		// OnUnequip_Script (last column)
		if (*p != '{')
		{
			ShowError("parse_item_constants_txt: Invalid format (OnUnequip_Script column) in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
			continue;
		}
		str[21] = p;
		p = &str[21][strlen(str[21]) - 2];

		if (*p != '}') {
			/* lets count to ensure it's not something silly e.g. a extra space at line ending */
			int lcurly = 0, rcurly = 0;

			for (size_t v = 0; v < strlen(str[21]); v++) {
				if (str[21][v] == '{')
					lcurly++;
				else if (str[21][v] == '}') {
					rcurly++;
					p = &str[21][v];
				}
			}

			if (lcurly != rcurly) {
				ShowError("parse_item_constants_txt: Mismatching curly braces in line %d of \"%s\" (item with id %d), skipping.\n", lines, path, atoi(str[0]));
				continue;
			}
		}
		str[21] = str[21] + 1;  //skip the first left curly
		*p = '\0';              //null the last right curly

		uint16 item_id = atoi( str[0] );
		char* name = trim( str[1] );

		aegis_itemnames[item_id] = std::string(name);

		if (atoi(str[14]) & (EQP_HELM | EQP_COSTUME_HELM) && util::umap_find(aegis_itemviewid, (t_itemid)atoi(str[18])) == nullptr)
			aegis_itemviewid[atoi(str[18])] = item_id;

		count++;
	}

	fclose(fp);

	ShowStatus("Done reading '" CL_WHITE "%u" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, path);

	return true;
}

const std::string ItemDatabase::getDefaultLocation(){
	return std::string( db_path ) + "/item_db.yml";
}

uint64 ItemDatabase::parseBodyNode(const YAML::Node& node) {
	t_itemid nameid;

	if (!this->asUInt32(node, "Id", nameid))
		return 0;

	if (this->nodeExists(node, "AegisName")) {
		std::string name;

		if (!this->asString(node, "AegisName", name))
			return 0;

		aegis_itemnames[nameid] = name;
	}

	if (this->nodeExists(node, "View")) {
		uint32 look;

		if (!this->asUInt32(node, "View", look))
			return 0;

		if( look > 0 ){
			if (this->nodeExists(node, "Locations")) {
				const YAML::Node& locationNode = node["Locations"];

				static std::vector<std::string> locations = {
					"Head_Low",
					"Head_Mid",
					"Head_Top",
					"Costume_Head_Low",
					"Costume_Head_Mid",
					"Costume_Head_Top"
				};

				for( std::string& location : locations ){
					if (this->nodeExists(locationNode, location)) {
						bool active;

						if (!this->asBool(locationNode, location, active))
							return 0;

						aegis_itemviewid[look] = nameid;
						break;
					}
				}
			}
		}
	}

	return 1;
}

void ItemDatabase::loadingFinished(){
}

ItemDatabase item_db;

static bool parse_mob_constants( char* split[], int columns, int current ){
	uint16 mob_id = atoi( split[0] );
	char* name = trim( split[1] );

	aegis_mobnames[mob_id] = std::string( name );

	return true;
}

static bool parse_skill_constants_txt( char* split[], int columns, int current ){
	uint16 skill_id = atoi( split[0] );
	char* name = trim( split[16] );

	aegis_skillnames[skill_id] = std::string( name );

	return true;
}

const std::string SkillDatabase::getDefaultLocation() {
	return std::string(db_path) + "/skill_db.yml";
}

uint64 SkillDatabase::parseBodyNode(const YAML::Node &node) {
	t_itemid nameid;

	if (!this->asUInt32(node, "Id", nameid))
		return 0;

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		aegis_skillnames[nameid] = name;
	}

	return 1;
}

void SkillDatabase::clear() {
	TypesafeCachedYamlDatabase::clear();
}

SkillDatabase skill_db;

/**
 * Split the string with ':' as separator and put each value for a skilllv
 * @param str: String to split
 * @param val: Array of MAX_SKILL_LEVEL to put value into
 * @return 0:error, x:number of value assign (should be MAX_SKILL_LEVEL)
 */
int skill_split_atoi(char *str, int *val) {
	int i;

	for (i = 0; i < MAX_SKILL_LEVEL; i++) {
		if (!str)
			break;
		val[i] = atoi(str);
		str = strchr(str, ':');
		if (str)
			*str++ = 0;
	}

	if (i == 0) // No data found.
		return 0;

	if (i == 1) // Single value, have the whole range have the same value.
		return 1;

	return i;
}

/**
 * Split string to int by constant value (const.txt) or atoi()
 * @param *str: String input
 * @param *val: Temporary storage
 * @param *delim: Delimiter (for multiple value support)
 * @param min_value: Minimum value. If the splitted value is less or equal than this, will be skipped
 * @param max: Maximum number that can be allocated
 * @return count: Number of success
 */
uint8 skill_split_atoi2(char *str, int64 *val, const char *delim, int min_value, uint16 max) {
	uint8 i = 0;
	char *p = strtok(str, delim);

	while (p != NULL) {
		int64 n = min_value;

		trim(p);

		if (ISDIGIT(p[0])) // If using numeric
			n = atoi(p);
		else {
			n = constant_lookup_int(p);
			p = strtok(NULL, delim);
		}

		if (n > min_value) {
			val[i] = n;
			i++;
			if (i >= max)
				break;
		}
		p = strtok(NULL, delim);
	}
	return i;
}

/**
 * Split string to int
 * @param str: String input
 * @param val1: Temporary storage to first value
 * @param val2: Temporary storage to second value
 */
static void itemdb_re_split_atoi(char* str, int* val1, int* val2) {
	int i, val[2];

	for (i = 0; i < 2; i++) {
		if (!str)
			break;
		val[i] = atoi(str);
		str = strchr(str, ':');
		if (str)
			*str++ = 0;
	}
	if (i == 0) {
		*val1 = *val2 = 0;
		return; // no data found
	}
	if (i == 1) { // Single Value
		*val1 = val[0];
		*val2 = 0;
		return;
	}

	// We assume we have 2 values.
	*val1 = val[0];
	*val2 = val[1];
	return;
}

// Implementation of the conversion functions

// Copied and adjusted from guild.cpp
// <skill id>,<max lv>,<req id1>,<req lv1>,<req id2>,<req lv2>,<req id3>,<req lv3>,<req id4>,<req lv4>,<req id5>,<req lv5>
static bool guild_read_guildskill_tree_db( char* split[], int columns, int current ){
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

		for (int i = 0, j = 0; i < MAX_GUILD_SKILL_REQUIRE; i++) {
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

	int lines = 0;
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

			if( p == NULL )
				break; // comma not found

			*p = '\0';
			++p;
		}

		if( p == NULL ) {
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

		if( p == NULL ) {
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
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' pets in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file );

	return true;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_magicmushroomdb(char *split[], int column, int current)
{
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
static bool skill_parse_row_abradb(char* split[], int columns, int current)
{
	uint16 skill_id = atoi(split[0]);
	std::string *skill_name = util::umap_find(aegis_skillnames, skill_id);

	if (skill_name == nullptr) {
		ShowError("Skill name for Abra skill ID %hu is not known.\n", skill_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Skill" << YAML::Value << *skill_name;

	int arr[MAX_SKILL_LEVEL];
	int arr_size = skill_split_atoi(split[2], arr);

	if (arr_size == 1) {
		if (arr[0] != 500)
			body << YAML::Key << "Probability" << YAML::Value << arr[0];
	} else {
		body << YAML::Key << "Probability";
		body << YAML::BeginSeq;

		for (int i = 0; i < arr_size; i++) {
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
static bool skill_parse_row_spellbookdb(char* split[], int columns, int current)
{
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
static bool mob_readdb_mobavail(char* str[], int columns, int current) {
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

		if (atoi(str[5]) != 0) {
			t_itemid weapon_item_id = strtoul( str[5], nullptr, 10 );
			std::string *weapon_item_name = util::umap_find(aegis_itemnames, weapon_item_id);

			if (weapon_item_name == nullptr) {
				ShowError("Item name for item ID %u (weapon) is not known.\n", weapon_item_id);
				return false;
			}

			body << YAML::Key << "Weapon" << YAML::Value << *weapon_item_name;
		}

		if (atoi(str[6]) != 0) {
			t_itemid shield_item_id = strtoul( str[6], nullptr, 10 );
			std::string *shield_item_name = util::umap_find(aegis_itemnames, shield_item_id);

			if (shield_item_name == nullptr) {
				ShowError("Item name for item ID %u (shield) is not known.\n", shield_item_id);
				return false;
			}

			body << YAML::Key << "Shield" << YAML::Value << *shield_item_name;
		}

		if (atoi(str[7]) != 0) {
			t_itemid *headtop_item_id = util::umap_find(aegis_itemviewid, (t_itemid)atoi(str[7]));

			if (headtop_item_id == nullptr) {
				ShowError("Item ID for view ID %hu (head top) is not known.\n", atoi(str[7]));
				return false;
			}

			std::string *headtop_item_name = util::umap_find(aegis_itemnames, (t_itemid)*headtop_item_id);

			if (headtop_item_name == nullptr) {
				ShowError("Item name for item ID %hu (head top) is not known.\n", *headtop_item_id);
				return false;
			}

			body << YAML::Key << "HeadTop" << YAML::Value << *headtop_item_name;
		}

		if (atoi(str[8]) != 0) {
			t_itemid *headmid_item_id = util::umap_find(aegis_itemviewid, (t_itemid)atoi(str[8]));

			if (headmid_item_id == nullptr) {
				ShowError("Item ID for view ID %hu (head mid) is not known.\n", atoi(str[8]));
				return false;
			}

			std::string *headmid_item_name = util::umap_find(aegis_itemnames, (t_itemid)*headmid_item_id);

			if (headmid_item_name == nullptr) {
				ShowError("Item name for item ID %hu (head mid) is not known.\n", *headmid_item_id);
				return false;
			}

			body << YAML::Key << "HeadMid" << YAML::Value << *headmid_item_name;
		}

		if (atoi(str[9]) != 0) {
			t_itemid *headlow_item_id = util::umap_find(aegis_itemviewid, (t_itemid)atoi(str[9]));

			if (headlow_item_id == nullptr) {
				ShowError("Item ID for view ID %hu (head low) is not known.\n", atoi(str[9]));
				return false;
			}

			std::string *headlow_item_name = util::umap_find(aegis_itemnames, (t_itemid)*headlow_item_id);

			if (headlow_item_name == nullptr) {
				ShowError("Item name for item ID %hu (head low) is not known.\n", *headlow_item_id);
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
static bool skill_parse_row_requiredb(char* split[], int columns, int current)
{
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
		int l = atoi(p);

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
		int l = atoi(p);

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
			for (int i = 0; i < count; i++) {
				entry.status.push_back((sc_type)require[i]);
			}
		}
	}

	skill_split_atoi(split[12], entry.spiritball);

	for (int i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++) {
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
			for (int i = 0; i < count; i++) {
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
static bool skill_parse_row_castdb(char* split[], int columns, int current)
{
	s_skill_db entry = {};

	skill_split_atoi(split[1], entry.cast);
	skill_split_atoi(split[2], entry.delay);
	skill_split_atoi(split[3], entry.walkdelay);
	skill_split_atoi(split[4], entry.upkeep_time);
	skill_split_atoi(split[5], entry.upkeep_time2);
	skill_split_atoi(split[6], entry.cooldown);
#ifdef RENEWAL_CAST
	skill_split_atoi(split[7], (int *)entry.fixed_cast);
#endif

	skill_cast.insert({ atoi(split[0]), entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_castnodexdb(char* split[], int columns, int current)
{
	s_skill_db entry = {};

	entry.castnodex = atoi(split[1]);
	if (split[2]) // optional column
		entry.delaynodex = atoi(split[2]);

	skill_castnodex.insert({ atoi(split[0]), entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_unitdb(char* split[], int columns, int current)
{
	s_skill_unit_csv entry = {};

	entry.unit_id = (uint16)strtol(split[1], NULL, 16);
	entry.unit_id2 = (uint16)strtol(split[2], NULL, 16);
	skill_split_atoi(split[3], entry.unit_layout_type);
	skill_split_atoi(split[4], entry.unit_range);
	entry.unit_interval = atoi(split[5]);
	entry.target_str = trim(split[6]);
	entry.unit_flag_csv = strtol(split[7], NULL, 16);

	skill_unit.insert({ atoi(split[0]), entry });

	return true;
}

// skill_db.yml function
//----------------------
static bool skill_parse_row_copyabledb(char* split[], int column, int current)
{
	s_skill_copyable entry = {};
	int skill_id = -1;

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
static bool skill_parse_row_nonearnpcrangedb(char* split[], int column, int current)
{
	s_skill_db entry = {};
	int skill_id = -1;

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

	skill_nearnpc.insert({ skill_id, entry });

	return true;
}

static bool isMultiLevel(int arr[]) {
	int count = 0;

	for (int i = 0; i < MAX_SKILL_LEVEL; i++) {
		if (arr[i] != 0)
			count++;
	}

	return (count == 0 || count == 1 ? false : true);
}

std::string name2Upper(std::string name) {
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	name[0] = toupper(name[0]);

	for (size_t i = 0; i < name.size(); i++) {
		if (name[i - 1] == '_' || (name[i - 2] == '1' && name[i - 1] == 'h') || (name[i - 2] == '2' && name[i - 1] == 'h'))
			name[i] = toupper(name[i]);
	}

	return name;
}

// Copied and adjusted from skill.cpp
static bool skill_parse_row_skilldb(char* split[], int columns, int current) {
	int arr[MAX_SKILL_LEVEL], arr_size, skill_id = atoi(split[0]);

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

	uint64 nk_val = strtol(split[5], NULL, 0);

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
		if (inf3_val & 0x20)
			body << YAML::Key << "IgnoreStasis" << YAML::Value << "true";
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

			for (int i = 0; i < arr_size; i++) {
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

			for (int i = 0; i < arr_size; i++) {
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

			for (int i = 0; i < arr_size; i++) {
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

			for (int i = 0; i < arr_size; i++) {
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

			for (int i = 0; i < arr_size; i++) {
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

			for (int i = 0; i < arr_size; i++) {
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

	if (strcmpi(split[9], "yes") == 0)
		body << YAML::Key << "CastCancel" << YAML::Value << "true";
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

			int temp = it_req->second.weapon;

			if (temp != 99) { // Not "All"
				for (int i = 0; i < MAX_WEAPON_TYPE_ALL; i++) {
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

			int temp = it_req->second.ammo;

			for (int i = 1; i < MAX_AMMO_TYPE; i++) {
				if (temp & 1 << i) {
					constant = constant_lookup(i, "A_");
					constant.erase(0, 2);
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
static bool quest_read_db(char *split[], int columns, int current) {
	int quest_id = atoi(split[0]);

	if (quest_id < 0 || quest_id >= INT_MAX) {
		ShowError("quest_read_db: Invalid quest ID '%d'.\n", quest_id);
		return false;
	}

	body << YAML::BeginMap;
	body << YAML::Key << "Id" << YAML::Value << quest_id;

	std::string title = split[17];
	
	if (columns > 18) { // If the title has a comma in it, concatenate
		int col = 18;

		while (col < columns) {
			title += ',' + std::string(split[col]);
			col++;
		}
	}

	title.erase(std::remove(title.begin(), title.end(), '"'), title.end()); // Strip double quotes out
	body << YAML::Key << "Title" << YAML::Value << title;

	if (strchr(split[1], ':') == NULL) {
		uint32 time = atoi(split[1]);

		if (time > 0) {
			int day = time / 86400;

			time %= (24 * 3600);
			int hour = time / 3600;

			time %= 3600;
			int minute = time / 60;

			time %= 60;
			int second = time;

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
static bool instance_readdb_sub(char* str[], int columns, int current) {
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

		for (int i = 7; i < columns; i++) {
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
static bool itemdb_read_itemavail(char *str[], int columns, int current) {
	item_avail.insert({ strtoul(str[0], nullptr, 10), strtoul(str[1], nullptr, 10) });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_buyingstore(char* fields[], int columns, int current) {
	item_buyingstore.insert({ strtoul(fields[0], nullptr, 10), true });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_flag(char* fields[], int columns, int current) {
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
static bool itemdb_read_itemdelay(char* str[], int columns, int current) {
	s_item_delay_csv2yaml item = { 0 };

	item.delay = atoi(str[1]);

	if (columns == 3)
		item.sc = trim(str[2]);

	item_delay.insert({ strtoul(str[0], nullptr, 10), item });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_stack(char* fields[], int columns, int current) {
	s_item_stack_csv2yaml item = { 0 };

	item.amount = atoi(fields[1]);

	int type = strtoul(fields[2], NULL, 10);

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
static bool itemdb_read_nouse(char* fields[], int columns, int current) {
	s_item_nouse_csv2yaml item = { 0 };

	item.sitting = "true";
	item.override = atoi(fields[2]);

	item_nouse.insert({ strtoul(fields[0], nullptr, 10), item });
	return true;
}

// item_db.yml function
//---------------------
static bool itemdb_read_itemtrade(char* str[], int columns, int current) {
	s_item_trade_csv2yaml item = { 0 };
	int flag = atoi(str[1]);

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

	int lines = 0;
	size_t entries = 0;
	char line[1024];

	while (fgets(line, sizeof(line), fp)) {
		char* str[32], * p;
		int i;

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
			if (p == NULL)
				break;// comma not found
			*p = '\0';
			++p;
		}

		if (p == NULL) {
			ShowError("itemdb_read_db: Insufficient columns in line %d (item with id %u), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}

		// Script
		if (*p != '{') {
			ShowError("itemdb_read_db: Invalid format (Script column) in line %d (item with id %u), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		str[19] = p + 1;
		p = strstr(p + 1, "},");
		if (p == NULL) {
			ShowError("itemdb_read_db: Invalid format (Script column) in line %d (item with id %u), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		*p = '\0';
		p += 2;

		// OnEquip_Script
		if (*p != '{') {
			ShowError("itemdb_read_db: Invalid format (OnEquip_Script column) in line %d (item with id %u), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		str[20] = p + 1;
		p = strstr(p + 1, "},");
		if (p == NULL) {
			ShowError("itemdb_read_db: Invalid format (OnEquip_Script column) in line %d (item with id %u), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		*p = '\0';
		p += 2;

		// OnUnequip_Script (last column)
		if (*p != '{') {
			ShowError("itemdb_read_db: Invalid format (OnUnequip_Script column) in line %d (item with id %u), skipping.\n", lines, strtoul(str[0], nullptr, 10));
			continue;
		}
		str[21] = p;
		p = &str[21][strlen(str[21]) - 2];

		if (*p != '}') {
			/* lets count to ensure it's not something silly e.g. a extra space at line ending */
			int lcurly = 0, rcurly = 0;

			for (size_t v = 0; v < strlen(str[21]); v++) {
				if (str[21][v] == '{')
					lcurly++;
				else if (str[21][v] == '}') {
					rcurly++;
					p = &str[21][v];
				}
			}

			if (lcurly != rcurly) {
				ShowError("itemdb_read_db: Mismatching curly braces in line %d (item with id %u), skipping.\n", lines, strtoul(str[0], nullptr, 10));
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

		int type = atoi(str[3]), subtype = atoi(str[18]);

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
		int atk = 0, matk = 0;

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
			uint64 temp_mask = strtoull(str[11], NULL, 0);

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

			int temp_class = atoi(str[12]);

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
			int temp_loc = atoi(str[14]);

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

		int elv = 0, elvmax = 0;

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
			body << YAML::Key << "Override" << YAML::Value << it_nouse->second.override;
			body << YAML::Key << "Sitting" << YAML::Value << "true";
			body << YAML::EndMap;
		}

		auto it_trade = item_trade.find(nameid);

		if (it_trade != item_trade.end()) {
			body << YAML::Key << "Trade";
			body << YAML::BeginMap;
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
static bool itemdb_read_randomopt_group(char* str[], int columns, int current) {
	if ((columns - 2) % 3 != 0) {
		ShowError("itemdb_read_randomopt_group: Invalid column entries '%d'.\n", columns);
		return false;
	}

	uint16 id = static_cast<uint16>(rand_opt_group.size() + 1);
	s_random_opt_group_csv *group = util::umap_find(rand_opt_group, id);
	s_random_opt_group_csv group_entry;

	if (group == nullptr)
		group_entry.rate.push_back((uint16)strtoul(str[1], nullptr, 10));

	for (int j = 0, k = 2; k < columns && j < MAX_ITEM_RDM_OPT; k += 3) {
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

		std::shared_ptr<s_random_opt_group_entry> entry;

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

static bool pc_readdb_levelpenalty( char* fields[], int columns, int current ){
	// 1=experience, 2=item drop
	int type = atoi( fields[0] );

	if( type != 1 && type != 2 ){
		ShowWarning( "pc_readdb_levelpenalty: Invalid type %d specified.\n", type );
		return false;
	}

	int64 val = constant_lookup_int( fields[1] );

	if( val == -100 ){
		ShowWarning("pc_readdb_levelpenalty: Unknown class constant %s specified.\n", fields[1] );
		return false;
	}

	int class_ = atoi( fields[1] );

	if( !CHK_CLASS( class_ ) ){
		ShowWarning( "pc_readdb_levelpenalty: Invalid class %d specified.\n", class_ );
		return false;
	}

	int diff = atoi( fields[2] );

	if( std::abs( diff ) > MAX_LEVEL ){
		ShowWarning( "pc_readdb_levelpenalty: Level difference %d is too high.\n", diff );
		return false;
	}

	diff += MAX_LEVEL - 1;

	level_penalty[type][class_][diff] = atoi(fields[3]);

	return true;
}

void pc_levelpenalty_yaml_sub( int type, const std::string& name ){
	body << YAML::BeginMap;
	body << YAML::Key << "Type" << YAML::Value << name;
	body << YAML::Key << "LevelDifferences";
	body << YAML::BeginSeq;
	for( int i = ARRAYLENGTH( level_penalty[type][CLASS_NORMAL] ); i >= 0; i-- ){
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

// Initialize Random Option constants
void init_random_option_constants() {
	#define export_constant2(a, b) script_set_constant_(a, b, a, false, false)

	export_constant2("RDMOPT_VAR_MAXHPAMOUNT", 1);
	export_constant2("RDMOPT_VAR_MAXSPAMOUNT", 2);
	export_constant2("RDMOPT_VAR_STRAMOUNT", 3);
	export_constant2("RDMOPT_VAR_AGIAMOUNT", 4);
	export_constant2("RDMOPT_VAR_VITAMOUNT", 5);
	export_constant2("RDMOPT_VAR_INTAMOUNT", 6);
	export_constant2("RDMOPT_VAR_DEXAMOUNT", 7);
	export_constant2("RDMOPT_VAR_LUKAMOUNT", 8);
	export_constant2("RDMOPT_VAR_MAXHPPERCENT", 9);
	export_constant2("RDMOPT_VAR_MAXSPPERCENT", 10);
	export_constant2("RDMOPT_VAR_HPACCELERATION", 11);
	export_constant2("RDMOPT_VAR_SPACCELERATION", 12);
	export_constant2("RDMOPT_VAR_ATKPERCENT", 13);
	export_constant2("RDMOPT_VAR_MAGICATKPERCENT", 14);
	export_constant2("RDMOPT_VAR_PLUSASPD", 15);
	export_constant2("RDMOPT_VAR_PLUSASPDPERCENT", 16);
	export_constant2("RDMOPT_VAR_ATTPOWER", 17);
	export_constant2("RDMOPT_VAR_HITSUCCESSVALUE", 18);
	export_constant2("RDMOPT_VAR_ATTMPOWER", 19);
	export_constant2("RDMOPT_VAR_ITEMDEFPOWER", 20);
	export_constant2("RDMOPT_VAR_MDEFPOWER", 21);
	export_constant2("RDMOPT_VAR_AVOIDSUCCESSVALUE", 22);
	export_constant2("RDMOPT_VAR_PLUSAVOIDSUCCESSVALUE", 23);
	export_constant2("RDMOPT_VAR_CRITICALSUCCESSVALUE", 24);
	export_constant2("RDMOPT_ATTR_TOLERACE_NOTHING", 25);
	export_constant2("RDMOPT_ATTR_TOLERACE_WATER", 26);
	export_constant2("RDMOPT_ATTR_TOLERACE_GROUND", 27);
	export_constant2("RDMOPT_ATTR_TOLERACE_FIRE", 28);
	export_constant2("RDMOPT_ATTR_TOLERACE_WIND", 29);
	export_constant2("RDMOPT_ATTR_TOLERACE_POISON", 30);
	export_constant2("RDMOPT_ATTR_TOLERACE_SAINT", 31);
	export_constant2("RDMOPT_ATTR_TOLERACE_DARKNESS", 32);
	export_constant2("RDMOPT_ATTR_TOLERACE_TELEKINESIS", 33);
	export_constant2("RDMOPT_ATTR_TOLERACE_UNDEAD", 34);
	export_constant2("RDMOPT_ATTR_TOLERACE_ALLBUTNOTHING", 35);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_NOTHING_USER", 36);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_NOTHING_TARGET", 37);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_WATER_USER", 38);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_WATER_TARGET", 39);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_GROUND_USER", 40);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_GROUND_TARGET", 41);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_FIRE_USER", 42);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_FIRE_TARGET", 43);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_WIND_USER", 44);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_WIND_TARGET", 45);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_POISON_USER", 46);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_POISON_TARGET", 47);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_SAINT_USER", 48);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_SAINT_TARGET", 49);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_DARKNESS_USER", 50);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_DARKNESS_TARGET", 51);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_TELEKINESIS_USER", 52);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_TELEKINESIS_TARGET", 53);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_UNDEAD_USER", 54);
	export_constant2("RDMOPT_DAMAGE_PROPERTY_UNDEAD_TARGET", 55);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_NOTHING_USER", 56);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_NOTHING_TARGET", 57);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_WATER_USER", 58);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_WATER_TARGET", 59);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_GROUND_USER", 60);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_GROUND_TARGET", 61);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_FIRE_USER", 62);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_FIRE_TARGET", 63);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_WIND_USER", 64);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_WIND_TARGET", 65);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_POISON_USER", 66);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_POISON_TARGET", 67);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_SAINT_USER", 68);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_SAINT_TARGET", 69);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_DARKNESS_USER", 70);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_DARKNESS_TARGET", 71);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_TELEKINESIS_USER", 72);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_TELEKINESIS_TARGET", 73);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_UNDEAD_USER", 74);
	export_constant2("RDMOPT_MDAMAGE_PROPERTY_UNDEAD_TARGET", 75);
	export_constant2("RDMOPT_BODY_ATTR_NOTHING", 76);
	export_constant2("RDMOPT_BODY_ATTR_WATER", 77);
	export_constant2("RDMOPT_BODY_ATTR_GROUND", 78);
	export_constant2("RDMOPT_BODY_ATTR_FIRE", 79);
	export_constant2("RDMOPT_BODY_ATTR_WIND", 80);
	export_constant2("RDMOPT_BODY_ATTR_POISON", 81);
	export_constant2("RDMOPT_BODY_ATTR_SAINT", 82);
	export_constant2("RDMOPT_BODY_ATTR_DARKNESS", 83);
	export_constant2("RDMOPT_BODY_ATTR_TELEKINESIS", 84);
	export_constant2("RDMOPT_BODY_ATTR_UNDEAD", 85);
	export_constant2("RDMOPT_RACE_TOLERACE_NOTHING", 87);
	export_constant2("RDMOPT_RACE_TOLERACE_UNDEAD", 88);
	export_constant2("RDMOPT_RACE_TOLERACE_ANIMAL", 89);
	export_constant2("RDMOPT_RACE_TOLERACE_PLANT", 90);
	export_constant2("RDMOPT_RACE_TOLERACE_INSECT", 91);
	export_constant2("RDMOPT_RACE_TOLERACE_FISHS", 92);
	export_constant2("RDMOPT_RACE_TOLERACE_DEVIL", 93);
	export_constant2("RDMOPT_RACE_TOLERACE_HUMAN", 94);
	export_constant2("RDMOPT_RACE_TOLERACE_ANGEL", 95);
	export_constant2("RDMOPT_RACE_TOLERACE_DRAGON", 96);
	export_constant2("RDMOPT_RACE_DAMAGE_NOTHING", 97);
	export_constant2("RDMOPT_RACE_DAMAGE_UNDEAD", 98);
	export_constant2("RDMOPT_RACE_DAMAGE_ANIMAL", 99);
	export_constant2("RDMOPT_RACE_DAMAGE_PLANT", 100);
	export_constant2("RDMOPT_RACE_DAMAGE_INSECT", 101);
	export_constant2("RDMOPT_RACE_DAMAGE_FISHS", 102);
	export_constant2("RDMOPT_RACE_DAMAGE_DEVIL", 103);
	export_constant2("RDMOPT_RACE_DAMAGE_HUMAN", 104);
	export_constant2("RDMOPT_RACE_DAMAGE_ANGEL", 105);
	export_constant2("RDMOPT_RACE_DAMAGE_DRAGON", 106);
	export_constant2("RDMOPT_RACE_MDAMAGE_NOTHING", 107);
	export_constant2("RDMOPT_RACE_MDAMAGE_UNDEAD", 108);
	export_constant2("RDMOPT_RACE_MDAMAGE_ANIMAL", 109);
	export_constant2("RDMOPT_RACE_MDAMAGE_PLANT", 110);
	export_constant2("RDMOPT_RACE_MDAMAGE_INSECT", 111);
	export_constant2("RDMOPT_RACE_MDAMAGE_FISHS", 112);
	export_constant2("RDMOPT_RACE_MDAMAGE_DEVIL", 113);
	export_constant2("RDMOPT_RACE_MDAMAGE_HUMAN", 114);
	export_constant2("RDMOPT_RACE_MDAMAGE_ANGEL", 115);
	export_constant2("RDMOPT_RACE_MDAMAGE_DRAGON", 116);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_NOTHING", 117);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_UNDEAD", 118);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_ANIMAL", 119);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_PLANT", 120);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_INSECT", 121);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_FISHS", 122);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_DEVIL", 123);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_HUMAN", 124);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_ANGEL", 125);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_DRAGON", 126);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_NOTHING", 127);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_UNDEAD", 128);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_ANIMAL", 129);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_PLANT", 130);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_INSECT", 131);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_FISHS", 132);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_DEVIL", 133);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_HUMAN", 134);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_ANGEL", 135);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_DRAGON", 136);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_NOTHING", 137);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_UNDEAD", 138);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_ANIMAL", 139);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_PLANT", 140);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_INSECT", 141);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_FISHS", 142);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_DEVIL", 143);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_HUMAN", 144);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_ANGEL", 145);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_DRAGON", 146);
	export_constant2("RDMOPT_CLASS_DAMAGE_NORMAL_TARGET", 147);
	export_constant2("RDMOPT_CLASS_DAMAGE_BOSS_TARGET", 148);
	export_constant2("RDMOPT_CLASS_DAMAGE_NORMAL_USER", 149);
	export_constant2("RDMOPT_CLASS_DAMAGE_BOSS_USER", 150);
	export_constant2("RDMOPT_CLASS_MDAMAGE_NORMAL", 151);
	export_constant2("RDMOPT_CLASS_MDAMAGE_BOSS", 152);
	export_constant2("RDMOPT_CLASS_IGNORE_DEF_PERCENT_NORMAL", 153);
	export_constant2("RDMOPT_CLASS_IGNORE_DEF_PERCENT_BOSS", 154);
	export_constant2("RDMOPT_CLASS_IGNORE_MDEF_PERCENT_NORMAL", 155);
	export_constant2("RDMOPT_CLASS_IGNORE_MDEF_PERCENT_BOSS", 156);
	export_constant2("RDMOPT_DAMAGE_SIZE_SMALL_TARGET", 157);
	export_constant2("RDMOPT_DAMAGE_SIZE_MIDIUM_TARGET", 158);
	export_constant2("RDMOPT_DAMAGE_SIZE_LARGE_TARGET", 159);
	export_constant2("RDMOPT_DAMAGE_SIZE_SMALL_USER", 160);
	export_constant2("RDMOPT_DAMAGE_SIZE_MIDIUM_USER", 161);
	export_constant2("RDMOPT_DAMAGE_SIZE_LARGE_USER", 162);
	export_constant2("RDMOPT_DAMAGE_SIZE_PERFECT", 163);
	export_constant2("RDMOPT_DAMAGE_CRI_TARGET", 164);
	export_constant2("RDMOPT_DAMAGE_CRI_USER", 165);
	export_constant2("RDMOPT_RANGE_ATTACK_DAMAGE_TARGET", 166);
	export_constant2("RDMOPT_RANGE_ATTACK_DAMAGE_USER", 167);
	export_constant2("RDMOPT_HEAL_VALUE", 168);
	export_constant2("RDMOPT_HEAL_MODIFY_PERCENT", 169);
	export_constant2("RDMOPT_DEC_SPELL_CAST_TIME", 170);
	export_constant2("RDMOPT_DEC_SPELL_DELAY_TIME", 171);
	export_constant2("RDMOPT_DEC_SP_CONSUMPTION", 172);
	export_constant2("RDMOPT_WEAPON_ATTR_NOTHING", 175);
	export_constant2("RDMOPT_WEAPON_ATTR_WATER", 176);
	export_constant2("RDMOPT_WEAPON_ATTR_GROUND", 177);
	export_constant2("RDMOPT_WEAPON_ATTR_FIRE", 178);
	export_constant2("RDMOPT_WEAPON_ATTR_WIND", 179);
	export_constant2("RDMOPT_WEAPON_ATTR_POISON", 180);
	export_constant2("RDMOPT_WEAPON_ATTR_SAINT", 181);
	export_constant2("RDMOPT_WEAPON_ATTR_DARKNESS", 182);
	export_constant2("RDMOPT_WEAPON_ATTR_TELEKINESIS", 183);
	export_constant2("RDMOPT_WEAPON_ATTR_UNDEAD", 184);
	export_constant2("RDMOPT_WEAPON_INDESTRUCTIBLE", 185);
	export_constant2("RDMOPT_BODY_INDESTRUCTIBLE", 186);
	export_constant2("RDMOPT_MDAMAGE_SIZE_SMALL_TARGET", 187);
	export_constant2("RDMOPT_MDAMAGE_SIZE_MIDIUM_TARGET", 188);
	export_constant2("RDMOPT_MDAMAGE_SIZE_LARGE_TARGET", 189);
	export_constant2("RDMOPT_MDAMAGE_SIZE_SMALL_USER", 190);
	export_constant2("RDMOPT_MDAMAGE_SIZE_MIDIUM_USER", 191);
	export_constant2("RDMOPT_MDAMAGE_SIZE_LARGE_USER", 192);
	export_constant2("RDMOPT_ATTR_TOLERACE_ALL", 193);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_NOTHING", 194);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_UNDEAD", 195);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_ANIMAL", 196);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_PLANT", 197);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_INSECT", 198);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_FISHS", 199);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_DEVIL", 200);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_HUMAN", 201);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_ANGEL", 202);
	export_constant2("RDMOPT_RACE_WEAPON_TOLERACE_DRAGON", 203);
	export_constant2("RDMOPT_RACE_TOLERACE_PLAYER_HUMAN", 206);
	export_constant2("RDMOPT_RACE_TOLERACE_PLAYER_DORAM", 207);
	export_constant2("RDMOPT_RACE_DAMAGE_PLAYER_HUMAN", 208);
	export_constant2("RDMOPT_RACE_DAMAGE_PLAYER_DORAM", 209);
	export_constant2("RDMOPT_RACE_MDAMAGE_PLAYER_HUMAN", 210);
	export_constant2("RDMOPT_RACE_MDAMAGE_PLAYER_DORAM", 211);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_PLAYER_HUMAN", 212);
	export_constant2("RDMOPT_RACE_CRI_PERCENT_PLAYER_DORAM", 213);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_PLAYER_HUMAN", 214);
	export_constant2("RDMOPT_RACE_IGNORE_DEF_PERCENT_PLAYER_DORAM", 215);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_PLAYER_HUMAN", 216);
	export_constant2("RDMOPT_RACE_IGNORE_MDEF_PERCENT_PLAYER_DORAM", 217);
	export_constant2("RDMOPT_MELEE_ATTACK_DAMAGE_TARGET", 219);
	export_constant2("RDMOPT_MELEE_ATTACK_DAMAGE_USER", 220);

	#undef export_constant2
}
