// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef YAML2SQL_HPP
#define YAML2SQL_HPP

#include <common/cbasetypes.hpp>
#include <common/core.hpp>
#include <fstream>
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <string>
#include <vector>

namespace rathena::tool_yaml2sql {

	struct ColumnDef {
		const char* sql_col;
		const char* yaml_key;
		const char* yaml_parent = nullptr;
		const char* yaml_fallback = nullptr;
		bool is_string = false;
	};

	struct FieldExtractor {
		const ColumnDef* def;
	};

	class Yaml2SqlTool : public server_core::Core {
		protected:
			bool initialize(int32 argc, char* argv[]) override;

		public:
			Yaml2SqlTool() : Core(server_core::e_core_type::TOOL) {}

		private:
			static bool askConfirmation(bool allow_all, const char* fmt, ...);
			static bool convert_to_sql(const std::string& file, std::ofstream& outFile, const std::string& table, const std::vector<FieldExtractor>& fields);
			static bool fileExists(const std::string& path);
			static void waitForAnyKey();

			template<size_t N> static bool db_yaml2sql(const std::string& file, std::ofstream& outFile, const std::string& table, const ColumnDef(&columns)[N]);
			template<typename Func> bool process(const std::string& type, uint32 version, const std::string& path, const std::string& name, const std::string& to_table, const std::string& table, Func lambda);

			static bool convert_all;
	};

	static const ColumnDef ITEM_COLUMNS[] = {
		// Root
		{"id", "Id"},
		{"name_aegis", "AegisName", nullptr, nullptr, true},
		{"name_english", "Name", nullptr, nullptr, true},
		{"type", "Type", nullptr, nullptr, true},
		{"subtype", "SubType", nullptr, nullptr, true},
		{"price_buy", "Buy"},
		{"price_sell", "Sell"},
		{"weight", "Weight"},
		{"attack", "Attack"},
	#ifdef RENEWAL
		{"magic_attack", "MagicAttack"},
	#endif
		{"defense", "Defense"},
		{"range", "Range"},
		{"slots", "Slots"},

		// Jobs
		{"job_all", "All", "Jobs"},
		{"job_acolyte", "Acolyte", "Jobs"},
		{"job_alchemist", "Alchemist", "Jobs"},
		{"job_archer", "Archer", "Jobs"},
		{"job_assassin", "Assassin", "Jobs"},
		{"job_barddancer", "BardDancer", "Jobs"},
		{"job_blacksmith", "Blacksmith", "Jobs"},
		{"job_crusader", "Crusader", "Jobs"},
		{"job_gunslinger", "Gunslinger", "Jobs"},
		{"job_hunter", "Hunter", "Jobs"},
	#ifdef RENEWAL
		{"job_kagerouoboro", "KagerouOboro", "Jobs"},
	#endif
		{"job_knight", "Knight", "Jobs"},
		{"job_mage", "Mage", "Jobs"},
		{"job_merchant", "Merchant", "Jobs"},
		{"job_monk", "Monk", "Jobs"},
		{"job_ninja", "Ninja", "Jobs"},
		{"job_novice", "Novice", "Jobs"},
		{"job_priest", "Priest", "Jobs"},
	#ifdef RENEWAL
		{"job_rebellion", "Rebellion", "Jobs"},
	#endif
		{"job_rogue", "Rogue", "Jobs"},
		{"job_sage", "Sage", "Jobs"},
		{"job_soullinker", "SoulLinker", "Jobs"},
	#ifdef RENEWAL
		{"job_spirit_handler", "Spirit_Handler", "Jobs"},
	#endif
		{"job_stargladiator", "StarGladiator", "Jobs"},
	#ifdef RENEWAL
		{"job_summoner", "Summoner", "Jobs"},
	#endif
		{"job_supernovice", "SuperNovice", "Jobs"},
		{"job_swordman", "Swordman", "Jobs"},
		{"job_taekwon", "Taekwon", "Jobs"},
		{"job_thief", "Thief", "Jobs"},
		{"job_wizard", "Wizard", "Jobs"},

		// Classes
		{"class_all", "All", "Classes"},
		{"class_normal", "Normal", "Classes"},
		{"class_upper", "Upper", "Classes", "All_Upper"},
		{"class_baby", "Baby", "Classes", "All_Baby"},
	#ifdef RENEWAL
		{"class_third", "Third", "Classes", "All_Third"},
		{"class_third_upper", "Third_Upper", "Classes", "All_Third"},
		{"class_third_baby", "Third_Baby", "Classes", "All_Third"},
		{"class_fourth", "Fourth", "Classes"},
	#endif

		// Root - Gender
		{"gender", "Gender", nullptr, nullptr, true},

		// Locations
		{"location_head_top", "Head_Top", "Locations"},
		{"location_head_mid", "Head_Mid", "Locations"},
		{"location_head_low", "Head_Low", "Locations"},
		{"location_armor", "Armor", "Locations"},
		{"location_left_hand", "Left_Hand", "Locations", "Both_Hand"},
		{"location_right_hand", "Right_Hand", "Locations", "Both_Hand"},
		{"location_garment", "Garment", "Locations"},
		{"location_shoes", "Shoes", "Locations"},
		{"location_right_accessory", "Right_Accessory", "Locations", "Both_Accessory"},
		{"location_left_accessory", "Left_Accessory", "Locations", "Both_Accessory"},
		{"location_costume_head_top", "Costume_Head_Top", "Locations"},
		{"location_costume_head_Mid", "Costume_Head_Mid", "Locations"},
		{"location_costume_head_Low", "Costume_Head_Low", "Locations"},
		{"location_costume_garment", "Costume_Garment", "Locations"},
		{"location_ammo", "Ammo", "Locations"},
		{"location_shadow_armor", "Shadow_Armor", "Locations"},
		{"location_shadow_weapon", "Shadow_Weapon", "Locations"},
		{"location_shadow_shield", "Shadow_Shield", "Locations"},
		{"location_shadow_shoes", "Shadow_Shoes", "Locations"},
		{"location_shadow_right_accessory", "Shadow_Right_Accessory", "Locations"},
		{"location_shadow_left_accessory", "Shadow_Left_Accessory", "Locations"},

		// Root - Equipment requirements
		{"weapon_level", "WeaponLevel"},
		{"armor_level", "ArmorLevel"},
		{"equip_level_min", "EquipLevelMin"},
		{"equip_level_max", "EquipLevelMax"},
		{"refineable", "Refineable"},
		{"gradable", "Gradable"},
		{"view", "View"},
		{"alias_name", "AliasName", nullptr, nullptr, true},

		// Flags
		{"flag_buyingstore", "BuyingStore", "Flags"},
		{"flag_deadbranch", "DeadBranch", "Flags"},
		{"flag_container", "Container", "Flags"},
		{"flag_uniqueid", "UniqueId", "Flags"},
		{"flag_bindonequip", "BindOnEquip", "Flags"},
		{"flag_dropannounce", "DropAnnounce", "Flags"},
		{"flag_noconsume", "NoConsume", "Flags"},
		{"flag_dropeffect", "DropEffect", "Flags", nullptr, true},

		// Delay
		{"delay_duration", "Duration", "Delay"},
		{"delay_status", "Status", "Delay", nullptr, true},

		// Stack
		{"stack_amount", "Amount", "Stack"},
		{"stack_inventory", "Inventory", "Stack"},
		{"stack_cart", "Cart", "Stack"},
		{"stack_storage", "Storage", "Stack"},
		{"stack_guildstorage", "GuildStorage", "Stack"},

		// NoUse
		{"nouse_override", "Override", "NoUse"},
		{"nouse_sitting", "Sitting", "NoUse"},

		// Trade
		{"trade_override", "Override", "Trade"},
		{"trade_nodrop", "NoDrop", "Trade"},
		{"trade_notrade", "NoTrade", "Trade"},
		{"trade_tradepartner", "TradePartner", "Trade"},
		{"trade_nosell", "NoSell", "Trade"},
		{"trade_nocart", "NoCart", "Trade"},
		{"trade_nostorage", "NoStorage", "Trade"},
		{"trade_noguildstorage", "NoGuildStorage", "Trade"},
		{"trade_nomail", "NoMail", "Trade"},
		{"trade_noauction", "NoAuction", "Trade"},

		// Root - Scripts
		{"script", "Script", nullptr, nullptr, true},
		{"equip_script", "EquipScript", nullptr, nullptr, true},
		{"unequip_script", "UnEquipScript", nullptr, nullptr, true}
	};

	static const ColumnDef MOB_COLUMNS[] = {
		{"id", "Id"},
		{"name_aegis", "AegisName", nullptr, nullptr, true},
		{"name_english", "Name", nullptr, nullptr, true},
		{"name_japanese", "Name", nullptr, nullptr, true},
		{"level", "Level"},
		{"hp", "Hp"},
		{"sp", "Sp"},
		{"base_exp", "BaseExp"},
		{"job_exp", "JobExp"},
		{"mvp_exp", "MvPExp"},
		{"attack", "Attack"},
		{"attack2", "Attack2"},
		{"defense", "Defense"},
		{"magic_defense", "MagicDefense"},
	#ifdef RENEWAL
		{"resistance", "Resistance"},
		{"magic_resistance", "MagicResistance"},
	#endif
		{"str", "Str"},
		{"agi", "Agi"},
		{"vit", "Vit"},
		{"int", "Int"},
		{"dex", "Dex"},
		{"luk", "Luk"},
		{"attack_range", "AttackRange"},
		{"skill_range", "SkillRange"},
		{"chase_range", "ChaseRange"},
		{"size", "Size", nullptr, nullptr, true},
		{"race", "Race", nullptr, nullptr, true},
		{"element", "Element", nullptr, nullptr, true},
		{"element_level", "ElementLevel"},
		{"walk_speed", "WalkSpeed"},
		{"attack_delay", "AttackDelay"},
		{"attack_motion", "AttackMotion"},
		{"damage_motion", "DamageMotion"},
		{"damage_taken", "DamageTaken"},
		{"groupid", "GroupId"},
		{"title", "Title", nullptr, nullptr, true},
		{"ai", "Ai", nullptr, nullptr, true},
		{"class", "Class", nullptr, nullptr, true},

		// Modes
		{"mode_canmove", "CanMove", "Modes"},
		{"mode_looter", "Looter", "Modes"},
		{"mode_aggressive", "Aggressive", "Modes"},
		{"mode_assist", "Assist", "Modes"},
		{"mode_castsensoridle", "CastSensorIdle", "Modes"},
		{"mode_norandomwalk", "NoRandomWalk", "Modes"},
		{"mode_nocast", "NoCast", "Modes"},
		{"mode_canattack", "CanAttack", "Modes"},
		{"mode_castsensorchase", "CastSensorChase", "Modes"},
		{"mode_changechase", "ChangeChase", "Modes"},
		{"mode_angry", "Angry", "Modes"},
		{"mode_changetargetmelee", "ChangeTargetMelee", "Modes"},
		{"mode_changetargetchase", "ChangeTargetChase", "Modes"},
		{"mode_targetweak", "TargetWeak", "Modes"},
		{"mode_randomtarget", "RandomTarget", "Modes"},
		{"mode_ignoremelee", "IgnoreMelee", "Modes"},
		{"mode_ignoremagic", "IgnoreMagic", "Modes"},
		{"mode_ignoreranged", "IgnoreRanged", "Modes"},
		{"mode_mvp", "Mvp", "Modes"},
		{"mode_ignoremisc", "IgnoreMisc", "Modes"},
		{"mode_knockbackimmune", "KnockBackImmune", "Modes"},
		{"mode_teleportblock", "TeleportBlock", "Modes"},
		{"mode_fixeditemdrop", "FixedItemDrop", "Modes"},
		{"mode_detector", "Detector", "Modes"},
		{"mode_statusimmune", "StatusImmune", "Modes"},
		{"mode_skillimmune", "SkillImmune", "Modes"}
	};

	static const std::string WHITESPACE = " \n\r\t\f\v";

	inline std::string ltrim(const std::string& s) {
		size_t start = s.find_first_not_of(WHITESPACE);

		return (start == std::string::npos) ? "" : s.substr(start);
	}

	inline std::string rtrim(const std::string& s) {
		size_t end = s.find_last_not_of(WHITESPACE);

		return (end == std::string::npos) ? "" : s.substr(0, end + 1);
	}

	inline std::string string_trim(const std::string& s) {
		return rtrim(ltrim(s));
	}

	inline std::string string_escape(const std::string& s) {
		size_t n = s.length();
		std::string escaped;
		escaped.reserve(n * 2);

		for (size_t i = 0; i < n; i++) {
			if (s[i] == '\r') continue;
			if (s[i] == '\n') {
				escaped += "\\n"; // For multiline scripts
				continue;
			}

			if (s[i] == '\\' || s[i] == '\'')
				escaped += '\\';

			escaped += s[i];
		}

		return escaped;
	}

	inline std::string name2Upper(std::string name) {
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		name[0] = toupper(name[0]);

		for (size_t i = 0; i < name.size(); i++) {
			if (name[i - 1] == '_' || (name[i - 2] == '1' && name[i - 1] == 'h') || (name[i - 2] == '2' && name[i - 1] == 'h'))
				name[i] = toupper(name[i]);
		}

		return name;
	}
}
#endif
