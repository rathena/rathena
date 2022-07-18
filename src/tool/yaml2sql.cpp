// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
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
#include <ryml_std.hpp>
#include <ryml.hpp>

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
#include "../map/elemental.hpp"
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

// Constants for conversion
std::unordered_map<const char *, int64> constants;

// Forward declaration of conversion functions
static bool item_db_yaml2sql(const std::string &file, const std::string &table);
static bool mob_db_yaml2sql(const std::string &file, const std::string &table);

bool fileExists( const std::string& path );
bool askConfirmation( const char* fmt, ... );

YAML::Node inNode;
std::ofstream outFile;

// Implement the function instead of including the original version by linking
void script_set_constant_(const char *name, int64 value, const char *constant_name, bool isparameter, bool deprecated) {
	constants[name] = value;
}

const char *constant_lookup(int32 value, const char *prefix) {
	if (prefix == nullptr)
		return nullptr;

	for (auto const &pair : constants) {
		// Same prefix group and same value
		if (strncasecmp(pair.first, prefix, strlen(prefix)) == 0 && pair.second == value) {
			return pair.first;
		}
	}

	return nullptr;
}

int64 constant_lookup_int(const char *constant) {
	if (constant == nullptr)
		return -100;

	for (auto const &pair : constants) {
		if (strlen(pair.first) == strlen(constant) && strncasecmp(pair.first, constant, strlen(constant)) == 0) {
			return pair.second;
		}
	}

	return -100;
}

void copyFileIfExists( std::ofstream& file,const std::string& name ){
	std::string path = "doc/yaml/sql/" + name + ".sql";

	if( fileExists( path ) ){
		std::ifstream source( path, std::ios::binary );

		std::istreambuf_iterator<char> begin_source(source);
		std::istreambuf_iterator<char> end_source;
		std::ostreambuf_iterator<char> begin_dest( file );
		copy( begin_source, end_source, begin_dest );

		source.close();
	}
}

void prepareHeader(std::ofstream &file, const std::string& name) {
	copyFileIfExists( file, name );

	file << "\n";
}

template<typename Func>
bool process( const std::string& type, uint32 version, const std::vector<std::string>& paths, const std::string& name, const std::string& to_table, const std::string& table, Func lambda ){
	for( const std::string& path : paths ){
		const std::string name_ext = name + ".yml";
		const std::string from = path + name_ext;
		const std::string to = "sql-files/" + to_table + ".sql";

		if( fileExists( from ) ){
#ifndef CONVERT_ALL
			if( !askConfirmation( "Found the file \"%s\", which can be converted to sql.\nDo you want to convert it now? (Y/N)\n", from.c_str() ) ){
				continue;
			}
#else
			ShowMessage("Found the file \"%s\", converting from yml to sql.\n", from.c_str());
#endif

			inNode.reset();

			try {
				inNode = YAML::LoadFile(from);
			} catch (YAML::Exception &e) {
				ShowError("%s (Line %d: Column %d)\n", e.msg.c_str(), e.mark.line, e.mark.column);
				if (!askConfirmation("Error found in \"%s\" while attempting to load.\nPress any key to continue.\n", from.c_str()))
					continue;
			}

			if (!inNode["Body"].IsDefined())
				continue;

#ifndef CONVERT_ALL
			if (fileExists(to)) {
				if (!askConfirmation("The file \"%s\" already exists.\nDo you want to replace it? (Y/N)\n", to.c_str())) {
					continue;
				}
			}
#endif

			outFile.open(to);

			if (!outFile.is_open()) {
				ShowError("Can not open file \"%s\" for writing.\n", to.c_str());
				return false;
			}

			prepareHeader(outFile, table.compare(to_table) == 0 ? table : to_table);

			if( !lambda( path, name_ext, table ) ){
				outFile.close();
				return false;
			}

			outFile.close();
		}
	}

	return true;
}

int do_init( int argc, char** argv ){
	const std::string path_db = std::string( db_path );
	const std::string path_db_mode = path_db + "/" + DBPATH;
	const std::string path_db_import = path_db + "/" + DBIMPORT + "/";
#ifdef RENEWAL
	const std::string item_table_name = "item_db_re";
	const std::string item_import_table_name = "item_db2_re";
	const std::string mob_table_name = "mob_db_re";
	const std::string mob_import_table_name = "mob_db2_re";
#else
	const std::string item_table_name = "item_db";
	const std::string item_import_table_name = "item_db2";
	const std::string mob_table_name = "mob_db";
	const std::string mob_import_table_name = "mob_db2";
#endif
	std::vector<std::string> item_table_suffixes = {
		"usable",
		"equip",
		"etc"
	};

	// Load constants
	#include "../map/script_constants.hpp"

	for( const std::string& suffix : item_table_suffixes ){
		if (!process("ITEM_DB", 1, { path_db_mode }, "item_db_" + suffix, item_table_name + "_" + suffix, item_table_name, [](const std::string& path, const std::string& name_ext, const std::string& table) -> bool {
			return item_db_yaml2sql(path + name_ext, table);
		})) {
			return 0;
		}
	}

	if (!process("ITEM_DB", 1, { path_db_import }, "item_db", item_import_table_name, item_import_table_name, [](const std::string& path, const std::string& name_ext, const std::string& table) -> bool {
		return item_db_yaml2sql(path + name_ext, table);
	})) {
		return 0;
	}

	if (!process("MOB_DB", 1, { path_db_mode }, "mob_db", mob_table_name, mob_table_name, [](const std::string &path, const std::string &name_ext, const std::string &table) -> bool {
		return mob_db_yaml2sql(path + name_ext, table);
	})) {
		return 0;
	}

	if (!process("MOB_DB", 1, { path_db_import }, "mob_db", mob_import_table_name, mob_import_table_name, [](const std::string &path, const std::string &name_ext, const std::string &table) -> bool {
		return mob_db_yaml2sql(path + name_ext, table);
	})) {
		return 0;
	}

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

std::string name2Upper(std::string name) {
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	name[0] = toupper(name[0]);

	for (size_t i = 0; i < name.size(); i++) {
		if (name[i - 1] == '_' || (name[i - 2] == '1' && name[i - 1] == 'h') || (name[i - 2] == '2' && name[i - 1] == 'h'))
			name[i] = toupper(name[i]);
	}

	return name;
}

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string &s) {
	size_t start = s.find_first_not_of(WHITESPACE);

	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s) {
	size_t end = s.find_last_not_of(WHITESPACE);

	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string string_trim(const std::string &s) {
	return rtrim(ltrim(s));
}

std::string string_escape(const std::string &s) {
	size_t n = s.length();
	std::string escaped;

	escaped.reserve(n * 2);

	for (size_t i = 0; i < n; ++i) {
		if (s[i] == '\r')
			continue;
		if (s[i] == '\n' && (i + 1) < n) {
			escaped += "\\n";
			continue;
		}
		if (s[i] == '\\' || s[i] == '\'')
			escaped += '\\';
		escaped += s[i];
	}

	return escaped;
}

/**
 * Append the entry value to the string if the Node exists
 * @param node: Node with entry
 * @param value: String to store node value to
 * @param string: If value is a string or not
 */
static bool appendEntry(const YAML::Node &node, std::string &value, bool string = false) {
	if (node.IsDefined()) {
		if (string) {
			value.append("'");
			value.append(string_trim(string_escape(node.as<std::string>())));
			value.append("',");
		} else {
			value.append(string_trim(node.as<std::string>()));
			value.append(",");
		}

		return true;
	}

	return false;
}

// Implementation of the conversion functions

// Copied and adjusted from itemdb.cpp
static bool item_db_yaml2sql(const std::string &file, const std::string &table) {
	size_t entries = 0;

	for (const YAML::Node &input : inNode["Body"]) {
		std::string column = "", value = "";

		if (appendEntry(input["Id"], value))
			column.append("`id`,");
		if (appendEntry(input["AegisName"], value, true))
			column.append("`name_aegis`,");
		if (appendEntry(input["Name"], value, true))
			column.append("`name_english`,");
		if (appendEntry(input["Type"], value, true))
			column.append("`type`,");
		if (appendEntry(input["SubType"], value, true))
			column.append("`subtype`,");
		if (appendEntry(input["Buy"], value))
			column.append("`price_buy`,");
		if (appendEntry(input["Sell"], value))
			column.append("`price_sell`,");
		if (appendEntry(input["Weight"], value))
			column.append("`weight`,");
		if (appendEntry(input["Attack"], value))
			column.append("`attack`,");
#ifdef RENEWAL
		if (appendEntry(input["MagicAttack"], value))
			column.append("`magic_attack`,");
#endif
		if (appendEntry(input["Defense"], value))
			column.append("`defense`,");
		if (appendEntry(input["Range"], value))
			column.append("`range`,");
		if (appendEntry(input["Slots"], value))
			column.append("`slots`,");

		const YAML::Node &jobs = input["Jobs"];

		if (jobs) {
			if (appendEntry(jobs["All"], value))
				column.append("`job_all`,");
			if (appendEntry(jobs["Acolyte"], value))
				column.append("`job_acolyte`,");
			if (appendEntry(jobs["Alchemist"], value))
				column.append("`job_alchemist`,");
			if (appendEntry(jobs["Archer"], value))
				column.append("`job_archer`,");
			if (appendEntry(jobs["Assassin"], value))
				column.append("`job_assassin`,");
			if (appendEntry(jobs["BardDancer"], value))
				column.append("`job_barddancer`,");
			if (appendEntry(jobs["Blacksmith"], value))
				column.append("`job_blacksmith`,");
			if (appendEntry(jobs["Crusader"], value))
				column.append("`job_crusader`,");
			if (appendEntry(jobs["Gunslinger"], value))
				column.append("`job_gunslinger`,");
			if (appendEntry(jobs["Hunter"], value))
				column.append("`job_hunter`,");
#ifdef RENEWAL
			if (appendEntry(jobs["KagerouOboro"], value))
				column.append("`job_kagerouoboro`,");
#endif
			if (appendEntry(jobs["Knight"], value))
				column.append("`job_knight`,");
			if (appendEntry(jobs["Mage"], value))
				column.append("`job_mage`,");
			if (appendEntry(jobs["Merchant"], value))
				column.append("`job_merchant`,");
			if (appendEntry(jobs["Monk"], value))
				column.append("`job_monk`,");
			if (appendEntry(jobs["Ninja"], value))
				column.append("`job_ninja`,");
			if (appendEntry(jobs["Novice"], value))
				column.append("`job_novice`,");
			if (appendEntry(jobs["Priest"], value))
				column.append("`job_priest`,");
#ifdef RENEWAL
			if (appendEntry(jobs["Rebellion"], value))
				column.append("`job_rebellion`,");
#endif
			if (appendEntry(jobs["Rogue"], value))
				column.append("`job_rogue`,");
			if (appendEntry(jobs["Sage"], value))
				column.append("`job_sage`,");
			if (appendEntry(jobs["SoulLinker"], value))
				column.append("`job_soullinker`,");
#ifdef RENEWAL
			if (appendEntry(jobs["Spirit_Handler"], value))
				column.append("`job_spirit_handler`,");
#endif
			if (appendEntry(jobs["StarGladiator"], value))
				column.append("`job_stargladiator`,");
#ifdef RENEWAL
			if (appendEntry(jobs["Summoner"], value))
				column.append("`job_summoner`,");
#endif
			if (appendEntry(jobs["SuperNovice"], value))
				column.append("`job_supernovice`,");
			if (appendEntry(jobs["Swordman"], value))
				column.append("`job_swordman`,");
			if (appendEntry(jobs["Taekwon"], value))
				column.append("`job_taekwon`,");
			if (appendEntry(jobs["Thief"], value))
				column.append("`job_thief`,");
			if (appendEntry(jobs["Wizard"], value))
				column.append("`job_wizard`,");
		}

		const YAML::Node &classes = input["Classes"];

		if (classes) {
			std::string str_all_upper;
			std::string str_all_baby;

			if (classes["All_Upper"].IsDefined())
				str_all_upper = string_trim(classes["All_Upper"].as<std::string>());
			if (classes["All_Baby"].IsDefined())
				str_all_baby = string_trim(classes["All_Baby"].as<std::string>());

			if (appendEntry(classes["All"], value))
				column.append("`class_all`,");
			if (appendEntry(classes["Normal"], value))
				column.append("`class_normal`,");
			if (appendEntry(classes["Upper"], value))
				column.append("`class_upper`,");
			else if (!str_all_upper.empty()) {
				value.append(str_all_upper);
				value.append(",");
				column.append("`class_upper`,");
			}
			if (appendEntry(classes["Baby"], value))
				column.append("`class_baby`,");
			else if (!str_all_baby.empty()) {
				value.append(str_all_baby);
				value.append(",");
				column.append("`class_baby`,");
			}
#ifdef RENEWAL
			std::string str_all_third;

			if (classes["All_Third"].IsDefined())
				str_all_third = string_trim(classes["All_Third"].as<std::string>());

			if (appendEntry(classes["Third"], value))
				column.append("`class_third`,");
			else if (!str_all_third.empty()) {
				value.append(str_all_third);
				value.append(",");
				column.append("`class_third`,");
			}
			if (appendEntry(classes["Third_Upper"], value))
				column.append("`class_third_upper`,");
			else if (!str_all_upper.empty() || !str_all_third.empty()) {
				if (!str_all_upper.empty())
					value.append(str_all_upper);
				else
					value.append(str_all_third);
				value.append(",");
				column.append("`class_third_upper`,");
			}
			if (appendEntry(classes["Third_Baby"], value))
				column.append("`class_third_baby`,");
			else if (!str_all_baby.empty() || !str_all_third.empty()) {
				if (!str_all_baby.empty())
					value.append(str_all_baby);
				else
					value.append(str_all_third);
				value.append(",");
				column.append("`class_third_baby`,");
			}
			if (appendEntry(classes["Fourth"], value))
				column.append("`class_fourth`,");
#endif
		}

		if (appendEntry(input["Gender"], value, true))
			column.append("`gender`,");

		const YAML::Node &locations = input["Locations"];

		if (locations) {
			if (appendEntry(locations["Head_Top"], value))
				column.append("`location_head_top`,");
			if (appendEntry(locations["Head_Mid"], value))
				column.append("`location_head_mid`,");
			if (appendEntry(locations["Head_Low"], value))
				column.append("`location_head_low`,");
			if (appendEntry(locations["Armor"], value))
				column.append("`location_armor`,");
			if (locations["Both_Hand"].IsDefined()) {
				std::string tmp_value = string_trim(locations["Both_Hand"].as<std::string>());
				if (!appendEntry(locations["Left_Hand"], value)) {
					value.append(tmp_value);
					value.append(",");
				}
				if (!appendEntry(locations["Right_Hand"], value)) {
					value.append(tmp_value);
					value.append(",");
				}
				column.append("`location_left_hand`,");
				column.append("`location_right_hand`,");

			}
			else {
				if (appendEntry(locations["Left_Hand"], value))
					column.append("`location_left_hand`,");
				if (appendEntry(locations["Right_Hand"], value))
					column.append("`location_right_hand`,");
			}
			if (appendEntry(locations["Garment"], value))
				column.append("`location_garment`,");
			if (appendEntry(locations["Shoes"], value))
				column.append("`location_shoes`,");
			if (locations["Both_Accessory"].IsDefined()) {
				std::string tmp_value = string_trim(locations["Both_Accessory"].as<std::string>());
				if (!appendEntry(locations["Right_Accessory"], value)) {
					value.append(tmp_value);
					value.append(",");
				}
				if (!appendEntry(locations["Left_Accessory"], value)) {
					value.append(tmp_value);
					value.append(",");
				}
				column.append("`location_right_accessory`,");
				column.append("`location_left_accessory`,");

			}
			else {
				if (appendEntry(locations["Right_Accessory"], value))
					column.append("`location_right_accessory`,");
				if (appendEntry(locations["Left_Accessory"], value))
					column.append("`location_left_accessory`,");
			}
			if (appendEntry(locations["Costume_Head_Top"], value))
				column.append("`location_costume_head_top`,");
			if (appendEntry(locations["Costume_Head_Mid"], value))
				column.append("`location_costume_head_Mid`,");
			if (appendEntry(locations["Costume_Head_Low"], value))
				column.append("`location_costume_head_Low`,");
			if (appendEntry(locations["Costume_Garment"], value))
				column.append("`location_costume_garment`,");
			if (appendEntry(locations["Ammo"], value))
				column.append("`location_ammo`,");
			if (appendEntry(locations["Shadow_Armor"], value))
				column.append("`location_shadow_armor`,");
			if (appendEntry(locations["Shadow_Weapon"], value))
				column.append("`location_shadow_weapon`,");
			if (appendEntry(locations["Shadow_Shield"], value))
				column.append("`location_shadow_shield`,");
			if (appendEntry(locations["Shadow_Shoes"], value))
				column.append("`location_shadow_shoes`,");
			if (appendEntry(locations["Shadow_Right_Accessory"], value))
				column.append("`location_shadow_right_accessory`,");
			if (appendEntry(locations["Shadow_Left_Accessory"], value))
				column.append("`location_shadow_left_accessory`,");
		}

		if (appendEntry(input["WeaponLevel"], value))
			column.append("`weapon_level`,");
		if (appendEntry(input["ArmorLevel"], value))
			column.append("`armor_level`,");
		if (appendEntry(input["EquipLevelMin"], value))
			column.append("`equip_level_min`,");
		if (appendEntry(input["EquipLevelMax"], value))
			column.append("`equip_level_max`,");
		if (appendEntry(input["Refineable"], value))
			column.append("`refineable`,");
		if (appendEntry(input["View"], value))
			column.append("`view`,");
		if (appendEntry(input["AliasName"], value, true))
			column.append("`alias_name`,");

		const YAML::Node &flags = input["Flags"];

		if (flags) {
			if (appendEntry(flags["BuyingStore"], value))
				column.append("`flag_buyingstore`,");
			if (appendEntry(flags["DeadBranch"], value))
				column.append("`flag_deadbranch`,");
			if (appendEntry(flags["Container"], value))
				column.append("`flag_container`,");
			if (appendEntry(flags["UniqueId"], value))
				column.append("`flag_uniqueid`,");
			if (appendEntry(flags["BindOnEquip"], value))
				column.append("`flag_bindonequip`,");
			if (appendEntry(flags["DropAnnounce"], value))
				column.append("`flag_dropannounce`,");
			if (appendEntry(flags["NoConsume"], value))
				column.append("`flag_noconsume`,");
			if (appendEntry(flags["DropEffect"], value, true))
				column.append("`flag_dropeffect`,");
		}

		const YAML::Node &delay = input["Delay"];

		if (delay) {
			if (appendEntry(delay["Duration"], value))
				column.append("`delay_duration`,");
			if (appendEntry(delay["Status"], value, true))
				column.append("`delay_status`,");
		}

		const YAML::Node &stack = input["Stack"];

		if (stack) {
			if (appendEntry(stack["Amount"], value))
				column.append("`stack_amount`,");
			if (appendEntry(stack["Inventory"], value))
				column.append("`stack_inventory`,");
			if (appendEntry(stack["Cart"], value))
				column.append("`stack_cart`,");
			if (appendEntry(stack["Storage"], value))
				column.append("`stack_storage`,");
			if (appendEntry(stack["GuildStorage"], value))
				column.append("`stack_guildstorage`,");
		}

		const YAML::Node &nouse = input["NoUse"];

		if (nouse) {
			if (appendEntry(nouse["Override"], value))
				column.append("`nouse_override`,");
			if (appendEntry(nouse["Sitting"], value))
				column.append("`nouse_sitting`,");
		}

		const YAML::Node &trade = input["Trade"];

		if (trade) {
			if (appendEntry(trade["Override"], value))
				column.append("`trade_override`,");
			if (appendEntry(trade["NoDrop"], value))
				column.append("`trade_nodrop`,");
			if (appendEntry(trade["NoTrade"], value))
				column.append("`trade_notrade`,");
			if (appendEntry(trade["TradePartner"], value))
				column.append("`trade_tradepartner`,");
			if (appendEntry(trade["NoSell"], value))
				column.append("`trade_nosell`,");
			if (appendEntry(trade["NoCart"], value))
				column.append("`trade_nocart`,");
			if (appendEntry(trade["NoStorage"], value))
				column.append("`trade_nostorage`,");
			if (appendEntry(trade["NoGuildStorage"], value))
				column.append("`trade_noguildstorage`,");
			if (appendEntry(trade["NoMail"], value))
				column.append("`trade_nomail`,");
			if (appendEntry(trade["NoAuction"], value))
				column.append("`trade_noauction`,");
		}

		if (appendEntry(input["Script"], value, true))
			column.append("`script`,");
		if (appendEntry(input["EquipScript"], value, true))
			column.append("`equip_script`,");
		if (appendEntry(input["UnEquipScript"], value, true))
			column.append("`unequip_script`,");

		column.pop_back(); // Remove last ','
		value.pop_back(); // Remove last ','

		outFile << "REPLACE INTO `" + table + "` (" + column + ") VALUES (" + value + ");\n";
		entries++;
	}

	ShowStatus("Done converting '" CL_WHITE "%zu" CL_RESET "' items in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}

// Copied and adjusted from mob.cpp
static bool mob_db_yaml2sql(const std::string &file, const std::string &table) {
	size_t entries = 0;

	for (const YAML::Node &input : inNode["Body"]) {
		std::string column = "", value = "";

		if (appendEntry(input["Id"], value))
			column.append("`id`,");
		if (appendEntry(input["AegisName"], value, true))
			column.append("`name_aegis`,");
		if (appendEntry(input["Name"], value, true))
			column.append("`name_english`,");
		if (appendEntry(input["Name"], value, true))
			column.append("`name_japanese`,");
		if (appendEntry(input["Level"], value))
			column.append("`level`,");
		if (appendEntry(input["Hp"], value))
			column.append("`hp`,");
		if (appendEntry(input["Sp"], value))
			column.append("`sp`,");
		if (appendEntry(input["BaseExp"], value))
			column.append("`base_exp`,");
		if (appendEntry(input["JobExp"], value))
			column.append("`job_exp`,");
		if (appendEntry(input["MvpExp"], value))
			column.append("`mvp_exp`,");
		if (appendEntry(input["Attack"], value))
			column.append("`attack`,");
		if (appendEntry(input["Attack2"], value))
			column.append("`attack2`,");
		if (appendEntry(input["Defense"], value))
			column.append("`defense`,");
		if (appendEntry(input["MagicDefense"], value))
			column.append("`magic_defense`,");
#ifdef RENEWAL
		if (appendEntry(input["Resistance"], value))
			column.append("`resistance`,");
		if (appendEntry(input["MagicResistance"], value))
			column.append("`magic_resistance`,");
#endif
		if (appendEntry(input["Str"], value))
			column.append("`str`,");
		if (appendEntry(input["Agi"], value))
			column.append("`agi`,");
		if (appendEntry(input["Vit"], value))
			column.append("`vit`,");
		if (appendEntry(input["Int"], value))
			column.append("`int`,");
		if (appendEntry(input["Dex"], value))
			column.append("`dex`,");
		if (appendEntry(input["Luk"], value))
			column.append("`luk`,");
		if (appendEntry(input["AttackRange"], value))
			column.append("`attack_range`,");
		if (appendEntry(input["SkillRange"], value))
			column.append("`skill_range`,");
		if (appendEntry(input["ChaseRange"], value))
			column.append("`chase_range`,");
		if (appendEntry(input["Size"], value, true))
			column.append("`size`,");
		if (appendEntry(input["Race"], value, true))
			column.append("`race`,");

		const YAML::Node &racegroups = input["RaceGroups"];

		if (racegroups) {
			for (uint16 i = 1; i < RC2_MAX; i++) {
				std::string constant = constant_lookup(i, "RC2_");

				constant.erase(0, 4);

				if (appendEntry(racegroups[name2Upper(constant)], value)) {
					std::transform(constant.begin(), constant.end(), constant.begin(), ::tolower);
					column.append("`racegroup_" + constant + "`,");
				}
			}
		}

		if (appendEntry(input["Element"], value, true))
			column.append("`element`,");
		if (appendEntry(input["ElementLevel"], value))
			column.append("`element_level`,");
		if (appendEntry(input["WalkSpeed"], value))
			column.append("`walk_speed`,");
		if (appendEntry(input["AttackDelay"], value))
			column.append("`attack_delay`,");
		if (appendEntry(input["AttackMotion"], value))
			column.append("`attack_motion`,");
		if (appendEntry(input["DamageMotion"], value))
			column.append("`damage_motion`,");
		if (appendEntry(input["DamageTaken"], value))
			column.append("`damage_taken`,");
		if (appendEntry(input["Ai"], value, true))
			column.append("`ai`,");
		if (appendEntry(input["Class"], value, true))
			column.append("`class`,");

		const YAML::Node &modes = input["Modes"];

		if (modes) {
			if (appendEntry(modes["CanMove"], value))
				column.append("`mode_canmove`,");
			if (appendEntry(modes["Looter"], value))
				column.append("`mode_looter`,");
			if (appendEntry(modes["Aggressive"], value))
				column.append("`mode_aggressive`,");
			if (appendEntry(modes["Assist"], value))
				column.append("`mode_assist`,");
			if (appendEntry(modes["CastSensorIdle"], value))
				column.append("`mode_castsensoridle`,");
			if (appendEntry(modes["NoRandomWalk"], value))
				column.append("`mode_norandomwalk`,");
			if (appendEntry(modes["NoCast"], value))
				column.append("`mode_nocast`,");
			if (appendEntry(modes["CanAttack"], value))
				column.append("`mode_canattack`,");
			if (appendEntry(modes["CastSensorChase"], value))
				column.append("`mode_castsensorchase`,");
			if (appendEntry(modes["ChangeChase"], value))
				column.append("`mode_changechase`,");
			if (appendEntry(modes["Angry"], value))
				column.append("`mode_angry`,");
			if (appendEntry(modes["ChangeTargetMelee"], value))
				column.append("`mode_changetargetmelee`,");
			if (appendEntry(modes["ChangeTargetChase"], value))
				column.append("`mode_changetargetchase`,");
			if (appendEntry(modes["TargetWeak"], value))
				column.append("`mode_targetweak`,");
			if (appendEntry(modes["RandomTarget"], value))
				column.append("`mode_randomtarget`,");
			if (appendEntry(modes["IgnoreMelee"], value))
				column.append("`mode_ignoremelee`,");
			if (appendEntry(modes["IgnoreMagic"], value))
				column.append("`mode_ignoremagic`,");
			if (appendEntry(modes["IgnoreRanged"], value))
				column.append("`mode_ignoreranged`,");
			if (appendEntry(modes["Mvp"], value))
				column.append("`mode_mvp`,");
			if (appendEntry(modes["IgnoreMisc"], value))
				column.append("`mode_ignoremisc`,");
			if (appendEntry(modes["KnockBackImmune"], value))
				column.append("`mode_knockbackimmune`,");
			if (appendEntry(modes["TeleportBlock"], value))
				column.append("`mode_teleportblock`,");
			if (appendEntry(modes["FixedItemDrop"], value))
				column.append("`mode_fixeditemdrop`,");
			if (appendEntry(modes["Detector"], value))
				column.append("`mode_detector`,");
			if (appendEntry(modes["StatusImmune"], value))
				column.append("`mode_statusimmune`,");
			if (appendEntry(modes["SkillImmune"], value))
				column.append("`mode_skillimmune`,");
		}

		for (uint16 i = 0; i < MAX_MVP_DROP; i++) {
			if (!input["MvpDrops"].IsDefined())
				continue;

			const YAML::Node &mvpdrops = input["MvpDrops"][i];

			if (mvpdrops) {
				if (appendEntry(mvpdrops["Item"], value, true))
					column.append("`mvpdrop" + std::to_string(i + 1) + "_item`,");
				if (appendEntry(mvpdrops["Rate"], value))
					column.append("`mvpdrop" + std::to_string(i + 1) + "_rate`,");
				if (appendEntry(mvpdrops["RandomOptionGroup"], value, true))
					column.append("`mvpdrop" + std::to_string(i + 1) + "_option`,");
				if (appendEntry(mvpdrops["Index"], value))
					column.append("`mvpdrop" + std::to_string(i + 1) + "_index`,");
			}
		}

		for (uint16 i = 0; i < MAX_MOB_DROP; i++) {
			if (!input["Drops"].IsDefined())
				continue;

			const YAML::Node &drops = input["Drops"][i];

			if (drops) {
				if (appendEntry(drops["Item"], value, true))
					column.append("`drop" + std::to_string(i + 1) + "_item`,");
				if (appendEntry(drops["Rate"], value))
					column.append("`drop" + std::to_string(i + 1) + "_rate`,");
				if (appendEntry(drops["StealProtected"], value))
					column.append("`drop" + std::to_string(i + 1) + "_nosteal`,");
				if (appendEntry(drops["RandomOptionGroup"], value, true))
					column.append("`drop" + std::to_string(i + 1) + "_option`,");
				if (appendEntry(drops["Index"], value))
					column.append("`drop" + std::to_string(i + 1) + "_index`,");
			}
		}

		column.pop_back(); // Remove last ','
		value.pop_back(); // Remove last ','

		outFile << "REPLACE INTO `" + table + "` (" + column + ") VALUES (" + value + ");\n";
		entries++;
	}

	ShowStatus("Done converting '" CL_WHITE "%zu" CL_RESET "' mobs in '" CL_WHITE "%s" CL_RESET "'.\n", entries, file.c_str());

	return true;
}
