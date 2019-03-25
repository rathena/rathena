// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../common/cbasetypes.hpp"
#include "../common/database.hpp"
#include "../common/mmo.hpp"

#include "script.hpp"

enum send_target : uint8;
struct block_list;

extern int16 instance_start;

#define MAX_INSTANCE_DATA 300 // Maximum amount of instances

#define INSTANCE_NAME_LENGTH (60+1)

enum e_instance_state : uint8 {
	INSTANCE_FREE,
	INSTANCE_IDLE,
	INSTANCE_BUSY
};

enum e_instance_mode : uint8 {
	IM_NONE,
	IM_CHAR,
	IM_PARTY,
	IM_GUILD,
	IM_CLAN,
	IM_MAX,
};

enum e_instance_enter : uint8 {
	IE_OK = 0,
	IE_NOMEMBER,
	IE_NOINSTANCE,
	IE_OTHER
};

struct s_instance_map {
	int16 m, src_m;
};

/// Instance data
struct s_instance_data {
	int id; ///< Instance DB ID
	enum e_instance_state state; ///< State of instance
	enum e_instance_mode mode; ///< Mode of instance
	int owner_id; ///< Owner ID of instance
	unsigned int keep_limit; ///< Life time of instance
	int keep_timer; ///< Remaining life time of instance
	unsigned int idle_limit; ///< Idle time of instance
	int idle_timer; ///< Remaining idle time of instance
	struct reg_db regs; ///< Instance variables for scripts
	std::vector<s_instance_map> map; ///< Array of maps in instance
};

/// Instance DB entry
struct s_instance_db {
	int id; ///< Instance ID
	std::string name; ///< Instance name
	unsigned int limit, ///< Duration limit
		timeout; ///< Timeout limit
	//bool destroyable; ///< Destroyable flag
	struct point enter; ///< Instance entry point
	std::vector<int16> maplist; ///< Maps in instance
};

class InstanceDatabase : public TypesafeYamlDatabase<uint32, s_instance_db> {
public:
	InstanceDatabase() : TypesafeYamlDatabase("INSTANCE_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node &node);
	//bool reload();
};

extern InstanceDatabase instance_db;

extern std::unordered_map<int, std::shared_ptr<s_instance_data>> instances;

std::shared_ptr<s_instance_data> instance_search(int instance_id);
std::shared_ptr<s_instance_db> instance_search_db_name(const char* name);
void instance_getsd(int instance_id, struct map_session_data *&sd, enum send_target *target);

int instance_create(int owner_id, const char *name, enum e_instance_mode mode);
bool instance_destroy(int instance_id);
enum e_instance_enter instance_enter(struct map_session_data *sd, int instance_id, const char *name, short x, short y);
bool instance_reqinfo(struct map_session_data *sd, int instance_id);
bool instance_addusers(int instance_id);
bool instance_delusers(int instance_id);
int16 instance_mapid(int16 m, int instance_id);
int instance_addmap(int instance_id);

void instance_addnpc(std::shared_ptr<s_instance_data> idata);

void instance_db_reload(void);
void do_reload_instance(void);
void do_init_instance(void);
void do_final_instance(void);

#endif /* INSTANCE_HPP */
