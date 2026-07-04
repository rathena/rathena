	/***********************************/
/***********    Shakto      ********/
/**    https://ronovelty.com/     **/
/***********************************/

#ifndef RUNE_HPP
#define RUNE_HPP

#include <map>
#include <vector>

#include "itemdb.hpp"

#include "../common/database.hpp"
#include "../common/db.hpp"
#include "../common/mmo.hpp"

enum e_runebook_result : uint16_t{
	ZC_RUNEBOOK_SUCCESS = 0,
	ZC_RUNEBOOK_ALRDYACTIVATED = 1,
	ZC_RUNEBOOK_NOTEXIST = 2,
	ZC_RUNEBOOK_NOITEM = 3,
};

enum e_runeset_result : uint16_t{
	ZC_RUNESET_SUCCESS = 0,
	ZC_RUNESET_NOITEM = 3,
	ZC_RUNESET_ALRDYACTIVATED = 4,
	ZC_RUNESET_NOEQUIPED = 5,
	ZC_RUNESET_TABLET_INVALID = 6,
	ZC_RUNESET_TABLET_MAXUPGRADE = 7,
	ZC_RUNESET_TABLET_INVALID2 = 8,
	ZC_RUNESET_NOMSG = 9,
	ZC_RUNESET_TABLET_INVALID3 = 10,
	ZC_RUNESET_TABLET_LOADING = 11,
	ZC_RUNESET_TABLET_INVALID4 = 12,
	ZC_RUNESET_TABLET_INVALID5 = 13,
};

enum e_runedecompo_result : uint8_t{
	ZC_RUNEDECOMPO_SUCCESS = 0,
	ZC_RUNEDECOMPO_UNKNOWN = 1,
	ZC_RUNEDECOMPO_NOCARD = 2,
	ZC_RUNEDECOMPO_INVENTORYSPACE = 3,
	ZC_RUNEDECOMPO_WEIGHT = 4,
	ZC_RUNEDECOMPO_MAXITEMS = 5,
	ZC_RUNEDECOMPO_UINOTOPENED = 6,
	ZC_RUNEDECOMPO_INVALID = 7,
	ZC_RUNEDECOMPO_NOTENOUGHITEMS = 8,
};

enum e_runereward_result : uint8_t {
	ZC_RUNEREWARD_SUCCESS = 0,
	ZC_RUNEREWARD_FAIL = 1,
	ZC_RUNEREWARD_INFONOTMATCH = 2,
	ZC_RUNEREWARD_INVENTORYSPACE = 3,
	ZC_RUNEREWARD_WEIGH = 4,
	ZC_RUNEREWARD_MAXITEMS = 5,
	ZC_RUNEREWARD_UINOTOPENED = 6,
	ZC_RUNEREWARD_INVALID = 7,
};

struct s_runedecompo_material_data {
	t_itemid material;
	uint32 amountmin;
	uint32 amountmax;
	uint32 chance;
};

struct s_runedecompo_db {
	uint32 id = 0;

	std::unordered_map<t_itemid, s_runedecompo_material_data> materials;
};

class RuneDecompoDatabase : public TypesafeYamlDatabase<uint32, s_runedecompo_db> {
public:
	RuneDecompoDatabase() : TypesafeYamlDatabase("RUNE_DECOMPO_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef &node) override;
};

struct s_runebook_db {
	uint32 id = 0;
	std::string name;
	std::unordered_map<t_itemid, uint16> materials;
};

class RuneBookDatabase : public TypesafeYamlDatabase<uint32, s_runebook_db> {
private:
	std::unordered_map<std::string, uint32> nameToBookId;
public:
	RuneBookDatabase() : TypesafeYamlDatabase("RUNEBOOK_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef &node) override;
	void clear() override{
		TypesafeYamlDatabase::clear();

		this->nameToBookId.clear();
	}

	uint32 search_name( const char *name );
};

struct s_upgrade_rune {
	uint16 grade = 0;
	uint32 chance = 0;
	uint32 chanceperfail = 0;
	std::unordered_map<t_itemid, uint16> materials;
};

struct s_script_rune {
	uint16 amount = 0;
	script_code *script;

	~s_script_rune() {
		if (this->script) {
			script_free_code(this->script);
			this->script = nullptr;
		}
	}
};

struct s_set_rune {
	uint32 id = 0;
	std::string name;
	std::unordered_map<uint16, uint32> slots; // slot, book id
	std::unordered_map<uint16, t_itemid> rewards;
	std::unordered_map<t_itemid, uint16> activation_materials;
	std::unordered_map<uint16, std::shared_ptr<s_script_rune>> scripts;
	std::unordered_map<uint16, std::shared_ptr<s_upgrade_rune>> upgrades;
};

struct s_rune_db {
	uint16 id;
	std::string name;

	std::unordered_map<uint32, std::shared_ptr<s_set_rune>> sets;
};

class RuneDatabase : public TypesafeYamlDatabase<uint16, s_rune_db> {
public:
	RuneDatabase() : TypesafeYamlDatabase("RUNE_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef &node) override;
};

extern RuneDecompoDatabase runedecompo_db;
extern RuneBookDatabase runebook_db;
extern RuneDatabase rune_db;

int32 rune_bookactivate(map_session_data* sd, uint16 tagID, uint32 runebookid);
int32 rune_setactivate(map_session_data* sd, uint16 tagID, uint32 runesetid);
std::tuple<uint8, uint16, uint16> rune_setupgrade(map_session_data* sd, uint16 tagID, uint32 runesetid);
bool rune_changestate(map_session_data* sd, uint16 tagID, uint32 runesetid);
void rune_count_bookactivated(map_session_data* sd, uint16 tagID, uint32 runesetid);
void rune_active_bonus(map_session_data *sd);

std::tuple<e_runereward_result, uint8> rune_askreward(map_session_data* sd, uint16 tagID, uint32 runesetid, uint8 reward);

void rune_save(map_session_data* sd);
void rune_load(map_session_data* sd);

void rune_db_reload();
void do_init_rune();
void do_final_rune();

#endif /* RUNE_HPP */
